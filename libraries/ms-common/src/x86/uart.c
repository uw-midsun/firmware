#include "uart.h"
#include "log.h"
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <pthread.h>

typedef struct {
  int sockfd;
  int listenfd;
  int connectfd;
  struct sockaddr_in addr;
  UARTStorage storage;
} SocketData;

static SocketData sock = {
  .sockfd = 0,
  .listenfd = 0,
  .connectfd = 0,
};

static void *accept_handler(void *unused);
static void *thread_handler(void *unused);

void *accept_handler(void *unused) {

  pthread_t receiver_thread;

  sock.connectfd = accept(sock.listenfd, (struct sockaddr*)NULL, NULL);
  close(sock.listenfd);

  sock.sockfd = sock.connectfd;

  if(pthread_create(&receiver_thread, NULL, thread_handler, NULL) < 0) {
    LOG_DEBUG("Error creating receiving thread.\n");
  }

  pthread_exit(NULL);
}

void *thread_handler(void *unused) {

  int n = 0;
  uint8_t buf[1024];
  int done = 0;

  while (!done) {

    n = read(sock.sockfd, buf, sizeof(buf)-1);

    if (n) {
      buf[n] = 0;
      sock.storage.rx_handler(buf, sizeof(buf), sock.storage.context);
    }

    if(buf[0] == 'z') {
      done = 1;
    }
  }

  return NULL;
}

StatusCode uart_init(UARTPort uart, UARTSettings *settings, UARTStorage *storage) {

  int one = 1;
  pthread_t receiver_thread;
  sock.storage.rx_handler = settings->rx_handler;
  sock.storage.context = settings->context;

  memset(&sock.addr, '0', sizeof(sock.addr));

  if((sock.sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
    LOG_DEBUG("Error: Could not create sock.\n");
    return status_code(STATUS_CODE_INTERNAL_ERROR);
  }

  sock.addr.sin_family = AF_INET;
  sock.addr.sin_addr.s_addr = htonl(INADDR_ANY);
  sock.addr.sin_port = htons(5000);

  if(connect(sock.sockfd, (struct sockaddr*)&sock.addr, sizeof(sock.addr)) < 0) {
    LOG_DEBUG("Could not connect to an open socket, listening for incoming connections.\n");

    sock.listenfd = socket(AF_INET, SOCK_STREAM, 0);
    memset(&sock.addr, '0', sizeof(sock.addr));

    sock.addr.sin_family = AF_INET;
    sock.addr.sin_addr.s_addr = htonl(INADDR_ANY);
    sock.addr.sin_port = htons(5000);
    int one = 1;
    setsockopt(sock.listenfd, SOL_SOCKET, SO_REUSEADDR, &one, sizeof(one));

    if(bind(sock.listenfd, (struct sockaddr*)&sock.addr, sizeof(sock.addr)) < 0) {
      LOG_DEBUG("Could not bind to an open sock.\n");
      return status_code(STATUS_CODE_INTERNAL_ERROR);
    }
    listen(sock.listenfd, 1);

    if(pthread_create(&receiver_thread, NULL, accept_handler, NULL) < 0) {
      LOG_DEBUG("Error creating receiving thread.\n");
      return status_code(STATUS_CODE_INTERNAL_ERROR);
    }

    return STATUS_CODE_UNINITIALIZED;

  } else {
    if(pthread_create(&receiver_thread, NULL, thread_handler, NULL) < 0) {
      LOG_DEBUG("Error creating receiving thread.\n");
      return status_code(STATUS_CODE_INTERNAL_ERROR);
    }
    return STATUS_CODE_OK;
  }
}

StatusCode uart_set_rx_handler(UARTPort uart, UARTRxHandler rx_handler, void *context) {
  sock.storage.rx_handler = rx_handler;
  sock.storage.context = context;

  return STATUS_CODE_OK;
}

StatusCode uart_tx(UARTPort uart, uint8_t *tx_data, size_t len) {

  if(write(sock.sockfd, tx_data, len) < 0) {
    LOG_DEBUG("Error sending data.\n");
    return status_code(STATUS_CODE_INTERNAL_ERROR);
  }

  return STATUS_CODE_OK;
}
