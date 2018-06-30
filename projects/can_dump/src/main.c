#include <inttypes.h>
#include "delay.h"
#include "generic_can.h"
#include "generic_can_hw.h"
#include "generic_can_mcp2515.h"
#include "gpio.h"
#include "gpio_it.h"
#include "interrupt.h"
#include "log.h"
#include "soft_timer.h"
#include "wait.h"

static GenericCanHw s_can;
static GenericCanMcp2515 s_can_mcp2515;

// static void prv_can_rx_callback(const GenericCanMsg *msg, void *context) {
//   printf("ID %" PRIx32 " DLC %d data 0x%lx%lx\n", msg->id, (uint8_t)msg->dlc,
//   (uint32_t)(msg->data >> 32), (uint32_t)msg->data);
// }

static void prv_mcp2515_rx(uint32_t id, bool extended, uint64_t data, size_t dlc, void *context) {
  printf("MCP2515: RX 0x%lx (dlc %d)\n", id, dlc);
}

int main(void) {
  interrupt_init();
  gpio_init();
  soft_timer_init();
  gpio_it_init();

  const CANHwSettings can_hw_settings = {
    .bitrate = CAN_HW_BITRATE_500KBPS,
    .tx = { GPIO_PORT_A, 12 },
    .rx = { GPIO_PORT_A, 11 },
    .loopback = false,
  };

  const Mcp2515Settings mcp2515_settings = {
    .spi_port = SPI_PORT_1,
    .baudrate = 750000,
    .mosi = { .port = GPIO_PORT_A, 7 },
    .miso = { .port = GPIO_PORT_A, 6 },
    .sclk = { .port = GPIO_PORT_A, 5 },
    .cs = { .port = GPIO_PORT_A, 4 },
    .int_pin = { .port = GPIO_PORT_A, 3 },

    .loopback = false,
  };
  generic_can_mcp2515_init(&s_can_mcp2515, &mcp2515_settings);
  generic_can_hw_init(&s_can, &can_hw_settings, 0);
  mcp2515_register_rx_cb(s_can_mcp2515.mcp2515, prv_mcp2515_rx, NULL);
  // generic_can_register_rx((GenericCan *)&s_can_mcp2515, prv_can_rx_callback, 0, 0, false, NULL);

  GPIOSettings led_settings = {
    .direction = GPIO_DIR_OUT,
    .state = GPIO_STATE_LOW,
  };
  GPIOAddress led = { .port = GPIO_PORT_A, .pin = 15 };
  gpio_init_pin(&led, &led_settings);

  printf("hello\n");

  while (true) {
    gpio_toggle_state(&led);
    printf("still alive\n");

    GenericCanMsg msg = {
      .id = 0x123,
      .data = 0,
      .dlc = 0,
    };
    generic_can_tx((GenericCan *)&s_can_mcp2515, &msg);

    // can_hw_transmit(0x123, false, NULL, 0);
    delay_s(1);
  }

  return 0;
}
