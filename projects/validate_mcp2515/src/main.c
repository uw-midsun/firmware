#include "generic_can_mcp2515.h"
#include "generic_can_hw.h"
#include "gpio.h"
#include "interrupt.h"
#include "soft_timer.h"
#include "gpio_it.h"
#include "log.h"

static GenericCanMcp2515 s_can_mcp2515;
static GenericCanHw s_can_hw;

static void prv_rx_can_hw(const GenericCanMsg *msg, void *context) {
  LOG_DEBUG("RX CAN HW: 0x%lx\n", msg->id);
}

static void prv_rx_can_mcp2515(const GenericCanMsg *msg, void *context) {
  LOG_DEBUG("RX MCP2515: 0x%lx\n", msg->id);
}

static void prv_periodic_tx(SoftTimerID timer_id, void *context) {
  static uint32_t i = 0;
  i = (i + 1) % 0x7FF;

  GenericCanMsg msg = {
    .id = i,
  };
  generic_can_tx((GenericCan *)&s_can_hw, &msg);
  generic_can_tx((GenericCan *)&s_can_mcp2515, &msg);

  // LOG_DEBUG("TX: 0x%lx\n", i);

  soft_timer_start_millis(5, prv_periodic_tx, NULL, NULL);
}

int main(void) {
  interrupt_init();
  gpio_init();
  gpio_it_init();
  soft_timer_init();
  event_queue_init();

  const CANHwSettings can_hw_settings = {
    .bitrate = CAN_HW_BITRATE_500KBPS,
    .tx = { GPIO_PORT_A, 12 },
    .rx = { GPIO_PORT_A, 11 },
    .loopback = false,
  };

  generic_can_hw_init(&s_can_hw, &can_hw_settings, 0);
  generic_can_register_rx((GenericCan *)&s_can_hw, prv_rx_can_hw, 0, 0, false, NULL);

  const Mcp2515Settings mcp2515_settings = {
    .spi_port = SPI_PORT_2,
    .spi_baudrate = 6000000,
    .mosi = { .port = GPIO_PORT_B, 15 },
    .miso = { .port = GPIO_PORT_B, 14 },
    .sclk = { .port = GPIO_PORT_B, 13 },
    .cs = { .port = GPIO_PORT_B, 12 },
    .int_pin = { .port = GPIO_PORT_A, 8 },

    .can_bitrate = MCP2515_BITRATE_500KBPS,
    .loopback = false,
  };

  generic_can_mcp2515_init(&s_can_mcp2515, &mcp2515_settings);
  generic_can_register_rx((GenericCan *)&s_can_hw, prv_rx_can_mcp2515, 0, 0, false, NULL);

  LOG_DEBUG("hello\n");
  soft_timer_start_millis(5, prv_periodic_tx, NULL, NULL);

  while (true) {
    mcp2515_poll(s_can_mcp2515.mcp2515);
  }

  return 0;
}