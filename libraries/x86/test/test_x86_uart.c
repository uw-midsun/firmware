#include <stdio.h>
#include <string.h>
#include "log.h"
#include "uart.h"

static void prv_receiver(const uint8_t *rx_arr, size_t len, void *context) {
  char arr[UART_MAX_BUFFER_LEN];
  strncpy(arr, (char *)rx_arr, len);
  LOG_DEBUG("%s\n", arr);
}

void setup_test(void) {}

void teardown_test(void) {}

void test_x86_uart_init(void) {
  char c = '0';
  char *send = "Test sentence.\n";

  UARTStorage storage1 = { .rx_handler = prv_receiver, .context = 0 };
  UARTStorage storage2 = { .rx_handler = prv_receiver, .context = 0 };
  UARTSettings settings = { .rx_handler = prv_receiver, .context = 0 };

  TEST_ASSERT_OK(uart_init(UART_PORT_1, &settings, &storage1));
  TEST_ASSERT_OK(uart_init(UART_PORT_2, &settings, &storage2));

  //
  //TEST_ASSERT_OK(uart_tx(UART_PORT_1, (uint8_t *)send, strlen(send)));
}
