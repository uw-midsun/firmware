#include <stdbool.h>

#include "button_led.h"
#include "button_led_radio.h"
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
  can_init(&s_can_storage, &can_settings);

  // Use I/O Expander for button LEDs
  I2CSettings settings = {
    .speed = I2C_SPEED_FAST,                   //
    .sda = CENTER_CONSOLE_CONFIG_PIN_I2C_SDA,  //
    .scl = CENTER_CONSOLE_CONFIG_PIN_I2C_SDL,  //
  };
  i2c_init(I2C_PORT_2, &settings);
  // Configure the expander to be output only
  gpio_expander_init(&s_expander, I2C_PORT_2, GPIO_EXPANDER_ADDRESS_0, NULL);

  // Initialize normal toggle buttons
  ButtonLedGpioExpanderPins expander_pins = {
    .bps_indicator = CENTER_CONSOLE_CONFIG_GPIO_EXPANDER_LED_BPS,
    .power_indicator = CENTER_CONSOLE_CONFIG_GPIO_EXPANDER_LED_POWER,
    .lights_drl = CENTER_CONSOLE_CONFIG_GPIO_EXPANDER_LED_DRL,
    .lights_low_beams = CENTER_CONSOLE_CONFIG_GPIO_EXPANDER_LED_LOW_BEAMS,
    .lights_hazards = CENTER_CONSOLE_CONFIG_GPIO_EXPANDER_LED_HAZARDS,
  };
  button_led_init(&s_expander, &expander_pins);

  // Initialize radio button groups
  ButtonLedRadioSettings radio_settings = {
    .reverse_pin = CENTER_CONSOLE_CONFIG_GPIO_EXPANDER_LED_REVERSE,
    .neutral_pin = CENTER_CONSOLE_CONFIG_GPIO_EXPANDER_LED_NEUTRAL,
    .drive_pin = CENTER_CONSOLE_CONFIG_GPIO_EXPANDER_LED_DRIVE,
  };
  button_led_radio_init(&s_expander, &radio_settings);

  Event e = { 0 };
  while (true) {
    while (status_ok(event_process(&e))) {
      can_process_event(&e);

      button_led_process_event(&e);
      button_led_radio_process_event(&e);
    }
  }
  return 0;
}
