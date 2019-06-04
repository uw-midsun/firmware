#include <stdbool.h>

#include "center_console_event.h"
#include "config.h"
#include "gpio.h"
#include "gpio_expander.h"
#include "gpio_it.h"
#include "i2c.h"
#include "interrupt.h"
#include "soft_timer.h"
#include "status.h"
#include "wait.h"

#include "can.h"
#include "can_msg_defs.h"
#include "can_transmit.h"
#include "can_unpack.h"

static CanStorage s_can_storage = { 0 };
static GpioExpanderStorage s_expander = { 0 };

int main() {
  // Standard module inits
  gpio_init();
  interrupt_init();
  gpio_it_init();
  soft_timer_init();
  event_queue_init();

  // CAN initialization
  const CanSettings can_settings = {
    .device_id = SYSTEM_CAN_DEVICE_DRIVER_CONTROLS_CENTER_CONSOLE,
    .bitrate = CAN_HW_BITRATE_500KBPS,
    .rx_event = CENTER_CONSOLE_EVENT_CAN_RX,
    .tx_event = CENTER_CONSOLE_EVENT_CAN_TX,
    .fault_event = CENTER_CONSOLE_EVENT_CAN_FAULT,
    .tx = CENTER_CONSOLE_CONFIG_PIN_CAN_TX,
    .rx = CENTER_CONSOLE_CONFIG_PIN_CAN_RX,
  };
  status_ok_or_return(can_init(&s_can_storage, &can_settings));

  // Use I/O Expander for button LEDs
  I2CSettings settings = {
    .speed = I2C_SPEED_FAST,                   //
    .sda = CENTER_CONSOLE_CONFIG_PIN_I2C_SDA,  //
    .scl = CENTER_CONSOLE_CONFIG_PIN_I2C_SDL,  //
  };
  status_ok_or_return(i2c_init(I2C_PORT_2, &settings));
  // Configure the expander to be output only
  status_ok_or_return(gpio_expander_init(&s_expander, I2C_PORT_2, GPIO_EXPANDER_ADDRESS_0, NULL));

  while (true) {
  }
  return 0;
}
