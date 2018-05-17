#include "uart.h"
#include "log.h"
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <pthread.h>

typedef struct {
  UARTPort port;
  int sockfd;
  int listenfd;
  int connectfd;
  struct sockaddr_in addr;
  UARTStorage storage;
} SocketData;

static SocketData *sock;
static unsigned long num_sock = 0;

static void *accept_handler(void *unused);
static void *thread_handler(void *unused);
static int get_sock(UARTPort uart);

void *accept_handler(void *port) {

  int current_sock = *((int*)port);
  pthread_t receiver_thread;
  sock[current_sock].connectfd = accept(sock[current_sock].listenfd, (struct sockaddr*)NULL, NULL);
  close(sock[current_sock].listenfd);

  sock[current_sock].sockfd = sock[current_sock].connectfd;

  if(pthread_create(&receiver_thread, NULL, thread_handler, port) < 0) {
    LOG_DEBUG("Error creating receiving thread.\n");
  }

  pthread_exit(NULL);
}

void *thread_handler(void *port) {

  int current_sock = *(int*)port;
  int n = 0;
  uint8_t buf[1024];
  int done = 0;

  while (!done) {

    n = read(sock[current_sock].sockfd, buf, sizeof(buf)-1);

    if (n) {
      buf[n] = 0;
      sock[current_sock].storage.rx_handler(buf, sizeof(buf), sock[current_sock].storage.context);
    }

    if(buf[0] == 'z') {
      done = 1;
    }
  }

  return NULL;
}

int get_sock(UARTPort uart) {

  for(unsigned long current_sock = 0; current_sock < num_sock; current_sock++) {
    if(sock[current_sock].port == uart) {
      return (int)current_sock;
    }
  }

  return -1;
}

StatusCode uart_init(UARTPort uart, UARTSettings *settings, UARTStorage *storage) {

  int one = 1;
  unsigned long latest_sock = num_sock;
  pthread_t receiver_thread;

  // append another SocketData struct to the array sock[]
  SocketData *tmp = sock;
  num_sock += 1;
  sock = (SocketData*)calloc(num_sock, sizeof(SocketData));
  memcpy(sock, tmp, sizeof(SocketData)*latest_sock);
  free(tmp);


  sock[latest_sock].port = uart;
  sock[latest_sock].storage.rx_handler = settings->rx_handler;
  sock[latest_sock].storage.context = settings->context;

  memset(&sock[latest_sock].addr, '0', sizeof(sock[latest_sock].addr));

  if((sock[latest_sock].sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
    LOG_DEBUG("Error: Could not create sock[uart].\n");
    return status_code(STATUS_CODE_INTERNAL_ERROR);
  }

  sock[latest_sock].addr.sin_family = AF_INET;
  sock[latest_sock].addr.sin_addr.s_addr = htonl(INADDR_ANY);
  sock[latest_sock].addr.sin_port = htons(5000);

  if(connect(sock[latest_sock].sockfd, (struct sockaddr*)&sock[latest_sock].addr, sizeof(sock[latest_sock].addr)) < 0) {
    LOG_DEBUG("Could not connect to an open socket, listening for incoming connections.\n");

    sock[latest_sock].listenfd = socket(AF_INET, SOCK_STREAM, 0);
    memset(&sock[latest_sock].addr, '0', sizeof(sock[latest_sock].addr));

    sock[latest_sock].addr.sin_family = AF_INET;
    sock[latest_sock].addr.sin_addr.s_addr = htonl(INADDR_ANY);
    sock[latest_sock].addr.sin_port = htons(5000);
    int one = 1;
    setsockopt(sock[latest_sock].listenfd, SOL_SOCKET, SO_REUSEADDR, &one, sizeof(one));

    if(bind(sock[latest_sock].listenfd, (struct sockaddr*)&sock[latest_sock].addr, sizeof(sock[latest_sock].addr)) < 0) {
      LOG_DEBUG("Could not bind to an open sock.\n");
      return status_code(STATUS_CODE_INTERNAL_ERROR);
    }
    listen(sock[latest_sock].listenfd, 1);

    if(pthread_create(&receiver_thread, NULL, accept_handler, (void*)&sock[latest_sock].port) < 0) {
      LOG_DEBUG("Error creating receiving thread.\n");
      return status_code(STATUS_CODE_INTERNAL_ERROR);
    }

    return STATUS_CODE_UNINITIALIZED;

  } else {
    if(pthread_create(&receiver_thread, NULL, thread_handler, (void*)&sock[latest_sock].port) < 0) {
      LOG_DEBUG("Error creating receiving thread.\n");
      return status_code(STATUS_CODE_INTERNAL_ERROR);
    }
    return STATUS_CODE_OK;
  }
}

StatusCode uart_set_rx_handler(UARTPort uart, UARTRxHandler rx_handler, void *context) {

  // check if the port has been initilized
  int current_sock;
  if((current_sock = get_sock(uart)) < 0) {
    LOG_DEBUG("UART Port %d has not been initialized.\n", uart);
    return status_code(STATUS_CODE_INTERNAL_ERROR);
  }

  sock[current_sock].storage.rx_handler = rx_handler;
  sock[current_sock].storage.context = context;

  return STATUS_CODE_OK;
}

StatusCode uart_tx(UARTPort uart, uint8_t *tx_data, size_t len) {

  // check if the port has been initilized
  int current_sock;
  if((current_sock = get_sock(uart)) < 0) {
    LOG_DEBUG("UART Port %d has not been initialized.\n", uart);
    return status_code(STATUS_CODE_INTERNAL_ERROR);
  }

  // check if a connection is open
  int n = 0;
  uint8_t buf[1];
  /*
  if((n = read(sock[current_sock].sockfd, buf, sizeof(buf))) < 0) {
    LOG_DEBUG("No port to write to.\n");
    return status_code(STATUS_CODE_INTERNAL_ERROR);
  }
  LOG_DEBUG("Can read %d\n.", n); */
  if(write(sock[current_sock].sockfd, tx_data, len) < 0) {
    LOG_DEBUG("Error sending data.\n");
    return status_code(STATUS_CODE_INTERNAL_ERROR);
  }

  return STATUS_CODE_OK;
}
