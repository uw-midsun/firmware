#include "can_hw.h"
#include <pthread.h>
#include <linux/can.h>
#include <linux/can/raw.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <poll.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include "log.h"

#define CAN_HW_DEV_INTERFACE "vcan0"
#define CAN_HW_MAX_FILTERS 14

typedef struct CANHwEventHandler {
  CANHwEventHandlerCb callback;
  void *context;
} CANHwEventHandler;

static pthread_t s_pthread_id;
static int s_can_fd = -1;
static struct can_frame s_frame = { 0 };
static struct can_filter s_filters[CAN_HW_MAX_FILTERS] = { 0 };
static size_t s_num_filters = 0;
static CANHwEventHandler s_handlers[NUM_CAN_HW_EVENTS] = { 0 };

static void *prv_rx_thread(void *arg) {
  while (true) {
    int bytes = read(s_can_fd, &s_frame, sizeof(s_frame));

    if (s_handlers[CAN_HW_EVENT_MSG_RX].callback != NULL) {
      printf("Running handler for %d\n", s_frame.can_id);
      s_handlers[CAN_HW_EVENT_MSG_RX].callback(s_handlers[CAN_HW_EVENT_TX_READY].context);
    }
  }

  return NULL;
}

StatusCode can_hw_init(const CANHwSettings *settings) {
  memset(s_handlers, 0, sizeof(s_handlers));
  memset(s_filters, 0, sizeof(s_filters));
  s_num_filters = 0;
  memset(&s_frame, 0, sizeof(s_frame));

  if (s_can_fd != -1) {
    // Reinit everything
    // TODO do this cleanly with signals or something
    pthread_cancel(s_pthread_id);
    close(s_can_fd);
  }

  s_can_fd = socket(PF_CAN, SOCK_RAW, CAN_RAW);
  if (s_can_fd == -1) {
    LOG_CRITICAL("CAN HW: Failed to open socketcan socket\n");
    return status_code(STATUS_CODE_INTERNAL_ERROR);
  }

  // Loopback - expects to receive its own messages
  int loopback = settings->loopback;
  setsockopt(s_can_fd, SOL_CAN_RAW, CAN_RAW_RECV_OWN_MSGS, &loopback, sizeof(loopback));

  struct ifreq ifr = { 0 };
  strcpy(ifr.ifr_name, CAN_HW_DEV_INTERFACE);
  ioctl(s_can_fd, SIOCGIFINDEX, &ifr);

  struct sockaddr_can addr = {
    .can_family = AF_CAN,
    .can_ifindex = ifr.ifr_ifindex,
  };
  bind(s_can_fd, (struct sockaddr *)&addr, sizeof(addr));

  pthread_create(&s_pthread_id, NULL, prv_rx_thread, NULL);

  return STATUS_CODE_OK;
}

// Registers a callback for the given event
StatusCode can_hw_register_callback(CANHwEvent event, CANHwEventHandlerCb callback, void *context) {
  if (event >= NUM_CAN_HW_EVENTS) {
    return status_code(STATUS_CODE_INVALID_ARGS);
  }

  s_handlers[event] = (CANHwEventHandler){
    .callback = callback,  //
    .context = context,    //
  };

  return STATUS_CODE_OK;
}

StatusCode can_hw_add_filter(uint16_t mask, uint16_t filter) {
  if (s_num_filters >= CAN_HW_MAX_FILTERS) {
    return status_msg(STATUS_CODE_RESOURCE_EXHAUSTED, "CAN HW: Ran out of filters.");
  }

  s_filters[s_num_filters].can_id = filter & CAN_SFF_MASK;
  s_filters[s_num_filters].can_mask = mask & CAN_SFF_MASK;
  s_num_filters++;

  setsockopt(s_can_fd, SOL_CAN_RAW, CAN_RAW_FILTER, s_filters, sizeof(s_filters[0]) * s_num_filters);

  return STATUS_CODE_OK;
}

CANHwBusStatus can_hw_bus_status(void) {
  return CAN_HW_BUS_STATUS_OK;
}

StatusCode can_hw_transmit(uint16_t id, const uint8_t *data, size_t len) {
  struct can_frame frame = {
    .can_id = id & CAN_SFF_MASK,
    .can_dlc = len
  };
  memcpy(frame.data, data, len);

  int bytes = write(s_can_fd, &frame, sizeof(frame));
  if (bytes < 0) {
    LOG_CRITICAL("CAN HW: Failed to TX frame\n");
    return status_code(STATUS_CODE_INTERNAL_ERROR);
  }

  printf("TX'd %d\n", id);
  if (s_handlers[CAN_HW_EVENT_TX_READY].callback != NULL) {
    s_handlers[CAN_HW_EVENT_TX_READY].callback(s_handlers[CAN_HW_EVENT_TX_READY].context);
  }

  return STATUS_CODE_OK;
}

// Must be called within the RX handler, returns whether a message was processed
bool can_hw_receive(uint16_t *id, uint64_t *data, size_t *len) {
  if (s_frame.can_id == 0) {
    return false;
  }

  *id = s_frame.can_id & CAN_SFF_MASK;
  memcpy(data, s_frame.data, sizeof(*data));
  *len = s_frame.can_dlc;

  memset(&s_frame, 0, sizeof(s_frame));
  return true;
}
