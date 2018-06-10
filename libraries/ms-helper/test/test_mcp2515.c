#include "gpio.h"
#include "gpio_it.h"
#include "interrupt.h"
#include "mcp2515.h"
#include "spi.h"
#include "unity.h"
#include "log.h"

static Mcp2515Storage s_mcp2515;
static volatile bool s_msg_rx = false;

static void prv_handle_rx(uint32_t id, bool extended, uint64_t data, size_t dlc, void *context) {
  LOG_DEBUG("RX id 0x%lx (extended %d) dlc %d data 0x%lx%lx\n", id, extended, dlc, (uint32_t)(data >> 32), (uint32_t)data);

  s_msg_rx = true;
}

void setup_test(void) {
  gpio_init();
  interrupt_init();
  gpio_it_init();

  s_msg_rx = false;

  const Mcp2515Settings mcp2515_settings = {
    .spi_port = SPI_PORT_1,
    .baudrate = 750000,
    .mosi = { .port = GPIO_PORT_A, 7 },
    .miso = { .port = GPIO_PORT_A, 6 },
    .sclk = { .port = GPIO_PORT_A, 5 },
    .cs = { .port = GPIO_PORT_A, 4 },

    .int_pin = { .port = GPIO_PORT_A, 3 },

    .loopback = true,

    .rx_cb = prv_handle_rx,
  };
  mcp2515_init(&s_mcp2515, &mcp2515_settings);
}

void teardown_test(void) {}

void test_mcp2515_loopback(void) {
  LOG_DEBUG("TX\n");
  mcp2515_tx(&s_mcp2515, 0x123, false, 0x1122334455667788, 8);

  while (!s_msg_rx) {}
}
