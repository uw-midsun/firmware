#include "can_hw.h"
#include <fcntl.h>
#include <linux/can.h>
#include <linux/can/raw.h>
#include <net/if.h>
#include <poll.h>
#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include "fifo.h"
#include "log.h"

#define CAN_HW_DEV_INTERFACE "vcan0"
#define CAN_HW_MAX_FILTERS 14
#define CAN_HW_TX_FIFO_LEN 8
// Check for thread exit once every 10ms
#define CAN_HW_THREAD_EXIT_PERIOD_US 10000

typedef struct CANHwEventHandler {
  CANHwEventHandlerCb callback;
  void *context;
} CANHwEventHandler;

typedef struct CANHwSocketData {
  int can_fd;
  struct can_frame rx_frame;
  Fifo tx_fifo;
  struct can_frame tx_frames[CAN_HW_TX_FIFO_LEN];
  struct can_filter filters[CAN_HW_MAX_FILTERS];
  size_t num_filters;
  CANHwEventHandler handlers[NUM_CAN_HW_EVENTS];
  uint32_t delay_us;
} CANHwSocketData;

static pthread_t s_rx_pthread_id;
static pthread_t s_tx_pthread_id;
static pthread_mutex_t s_tx_mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t s_tx_cond = PTHREAD_COND_INITIALIZER;

static pthread_mutex_t s_should_exit = PTHREAD_MUTEX_INITIALIZER;

static CANHwSocketData s_socket_data = {.can_fd = -1 };

static uint32_t prv_get_delay(CANHwBitrate bitrate) {
  const uint32_t delay_us[NUM_CAN_HW_BITRATES] = {
    1000,  // 125 kbps
    500,   // 250 kbps
    250,   // 500 kbps
    125,   // 1 mbps
  };

  return delay_us[bitrate];
}

static void *prv_rx_thread(void *arg) {
  LOG_DEBUG("CAN HW RX thread started\n");

  struct timeval timeout = {.tv_usec = CAN_HW_THREAD_EXIT_PERIOD_US };

  // Mutex is locked when the thread should exit
  while (pthread_mutex_trylock(&s_should_exit) == 0) {
    pthread_mutex_unlock(&s_should_exit);

    // Select timeout is used to poll every now and then
    fd_set input_fds;
    FD_ZERO(&input_fds);
    FD_SET(s_socket_data.can_fd, &input_fds);

    select(s_socket_data.can_fd + 1, &input_fds, NULL, NULL, &timeout);

    if (FD_ISSET(s_socket_data.can_fd, &input_fds)) {
      int bytes =
          read(s_socket_data.can_fd, &s_socket_data.rx_frame, sizeof(s_socket_data.rx_frame));

      if (s_socket_data.handlers[CAN_HW_EVENT_MSG_RX].callback != NULL) {
        s_socket_data.handlers[CAN_HW_EVENT_MSG_RX].callback(
            s_socket_data.handlers[CAN_HW_EVENT_TX_READY].context);
      }

      // Limit how often we can receive messages to simulate bus speed
      usleep(s_socket_data.delay_us);
    }
  }

  return NULL;
}

static void *prv_tx_thread(void *arg) {
  LOG_DEBUG("CAN HW TX thread started\n");
  struct can_frame frame = { 0 };

  // Mutex is locked when the thread should exit
  while (pthread_mutex_trylock(&s_should_exit) == 0) {
    pthread_mutex_unlock(&s_should_exit);

    pthread_mutex_lock(&s_tx_mutex);
    while (fifo_size(&s_socket_data.tx_fifo) == 0) {
      pthread_cond_wait(&s_tx_cond, &s_tx_mutex);

      if (pthread_mutex_trylock(&s_should_exit) != 0) {
        break;
      }
      pthread_mutex_lock(&s_should_exit);
    }

    fifo_pop(&s_socket_data.tx_fifo, &frame);
    pthread_mutex_unlock(&s_tx_mutex);

    int bytes = write(s_socket_data.can_fd, &frame, sizeof(frame));

    // Delay to simulate bus speed
    usleep(s_socket_data.delay_us);

    if (s_socket_data.handlers[CAN_HW_EVENT_TX_READY].callback != NULL) {
      s_socket_data.handlers[CAN_HW_EVENT_TX_READY].callback(
          s_socket_data.handlers[CAN_HW_EVENT_TX_READY].context);
    }
  }

  return NULL;
}

StatusCode can_hw_init(const CANHwSettings *settings) {
  if (s_socket_data.can_fd != -1) {
    // Request threads to exit
    close(s_socket_data.can_fd);

    pthread_mutex_lock(&s_should_exit);

    // Signal condition variable in case TX thread is waiting
    pthread_mutex_lock(&s_tx_mutex);
    pthread_cond_signal(&s_tx_cond);
    pthread_mutex_unlock(&s_tx_mutex);

    pthread_join(s_tx_pthread_id, NULL);
    pthread_join(s_rx_pthread_id, NULL);
    pthread_mutex_unlock(&s_should_exit);
  }

  pthread_mutex_init(&s_should_exit, NULL);
  pthread_mutex_init(&s_tx_mutex, NULL);
  pthread_cond_init(&s_tx_cond, NULL);

  memset(&s_socket_data, 0, sizeof(s_socket_data));
  s_socket_data.delay_us = prv_get_delay(settings->bitrate);
  fifo_init(&s_socket_data.tx_fifo, s_socket_data.tx_frames);

  s_socket_data.can_fd = socket(PF_CAN, SOCK_RAW, CAN_RAW);
  if (s_socket_data.can_fd == -1) {
    LOG_CRITICAL("CAN HW: Failed to open SocketCAN socket\n");
    return status_msg(STATUS_CODE_INTERNAL_ERROR, "CAN HW: Failed to open socket");
  }

  // Loopback - expects to receive its own messages
  int loopback = settings->loopback;
  if (setsockopt(s_socket_data.can_fd, SOL_CAN_RAW, CAN_RAW_RECV_OWN_MSGS, &loopback,
                 sizeof(loopback)) < 0) {
    LOG_CRITICAL("CAN HW: Failed to set loopback mode on socket\n");
    return status_msg(STATUS_CODE_INTERNAL_ERROR, "CAN HW: Failed to set loopback mode on socket");
  }

  struct ifreq ifr = { 0 };
  snprintf(ifr.ifr_name, sizeof(ifr.ifr_name), "%s", CAN_HW_DEV_INTERFACE);
  if (ioctl(s_socket_data.can_fd, SIOCGIFINDEX, &ifr) < 0) {
    LOG_CRITICAL("CAN HW: Device %s not found\n", CAN_HW_DEV_INTERFACE);
    return status_msg(STATUS_CODE_INTERNAL_ERROR, "CAN HW: Device not found");
  }

  // Set non-blocking
  fcntl(s_socket_data.can_fd, F_SETFL, O_NONBLOCK);

  struct sockaddr_can addr = {
    .can_family = AF_CAN, .can_ifindex = ifr.ifr_ifindex,
  };
  if (bind(s_socket_data.can_fd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
    LOG_CRITICAL("CAN HW: Failed to bind socket\n");
    return status_msg(STATUS_CODE_INTERNAL_ERROR, "CAN HW: Failed to bind socket");
  }

  LOG_DEBUG("CAN HW initialized on %s\n", CAN_HW_DEV_INTERFACE);

  pthread_create(&s_rx_pthread_id, NULL, prv_rx_thread, NULL);
  pthread_create(&s_tx_pthread_id, NULL, prv_tx_thread, NULL);

  return STATUS_CODE_OK;
}

// Registers a callback for the given event
StatusCode can_hw_register_callback(CANHwEvent event, CANHwEventHandlerCb callback, void *context) {
  if (event >= NUM_CAN_HW_EVENTS) {
    return status_code(STATUS_CODE_INVALID_ARGS);
  }

  s_socket_data.handlers[event] = (CANHwEventHandler){
    .callback = callback,  //
    .context = context,    //
  };

  return STATUS_CODE_OK;
}

StatusCode can_hw_add_filter(uint16_t mask, uint16_t filter) {
  if (s_socket_data.num_filters >= CAN_HW_MAX_FILTERS) {
    return status_msg(STATUS_CODE_RESOURCE_EXHAUSTED, "CAN HW: Ran out of filters.");
  }

  s_socket_data.filters[s_socket_data.num_filters].can_id = filter & CAN_SFF_MASK;
  s_socket_data.filters[s_socket_data.num_filters].can_mask = mask & CAN_SFF_MASK;
  s_socket_data.num_filters++;

  if (setsockopt(s_socket_data.can_fd, SOL_CAN_RAW, CAN_RAW_FILTER, s_socket_data.filters,
                 sizeof(s_socket_data.filters[0]) * s_socket_data.num_filters) < 0) {
    return status_msg(STATUS_CODE_INTERNAL_ERROR, "CAN HW: Failed to set raw filters");
  }

  return STATUS_CODE_OK;
}

CANHwBusStatus can_hw_bus_status(void) {
  return CAN_HW_BUS_STATUS_OK;
}

StatusCode can_hw_transmit(uint16_t id, const uint8_t *data, size_t len) {
  struct can_frame frame = {.can_id = id & CAN_SFF_MASK, .can_dlc = len };
  memcpy(&frame.data, data, len);

  pthread_mutex_lock(&s_tx_mutex);
  StatusCode ret = fifo_push(&s_socket_data.tx_fifo, &frame);
  if (ret != STATUS_CODE_OK) {
    // Fifo is full
    pthread_mutex_unlock(&s_tx_mutex);
    return status_msg(STATUS_CODE_RESOURCE_EXHAUSTED, "CAN HW TX failed");
  }

  pthread_cond_signal(&s_tx_cond);
  pthread_mutex_unlock(&s_tx_mutex);

  return STATUS_CODE_OK;
}

// Must be called within the RX handler, returns whether a message was processed
bool can_hw_receive(uint16_t *id, uint64_t *data, size_t *len) {
  if (s_socket_data.rx_frame.can_id == 0) {
    // Assumes that we'll never transmit something with a CAN ID of all 0s
    return false;
  }

  *id = s_socket_data.rx_frame.can_id & CAN_SFF_MASK;
  memcpy(data, s_socket_data.rx_frame.data, sizeof(*data));
  *len = s_socket_data.rx_frame.can_dlc;

  memset(&s_socket_data.rx_frame, 0, sizeof(s_socket_data.rx_frame));
  return true;
}
