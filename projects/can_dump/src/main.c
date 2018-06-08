#include "delay.h"
#include "generic_can.h"
#include "generic_can_hw.h"
#include "gpio.h"
#include "interrupt.h"
#include "log.h"
#include "soft_timer.h"
#include "wait.h"

static GenericCanHw s_can;

static void prv_can_rx_callback(const GenericCanMsg *msg, void *context) {
  printf("ID %ld DLC %u extended %d\n", msg->id, msg->dlc, msg->extended);
}

int main(void) {
  interrupt_init();
  gpio_init();
  soft_timer_init();

  const CANHwSettings can_hw_settings = {
    .bitrate = CAN_HW_BITRATE_250KBPS,
    .tx = { GPIO_PORT_A, 12 },
    .rx = { GPIO_PORT_A, 11 },
    .loopback = false,
  };

  generic_can_hw_init(&s_can, &can_hw_settings, 0);
  generic_can_register_rx((GenericCan *)&s_can, prv_can_rx_callback, 0, 0, true, NULL);
  generic_can_register_rx((GenericCan *)&s_can, prv_can_rx_callback, 0, 0, false, NULL);

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
    can_hw_transmit(0x123, false, NULL, 0);
    delay_s(1);
  }

  return 0;
}
