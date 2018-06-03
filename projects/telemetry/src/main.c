#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include "log.h"

#include "delay.h"
#include "gpio.h"  // General Purpose I/O control.
#include "gps.h"
#include "interrupt.h"   // For enabling interrupts.
#include "soft_timer.h"  // Software timers for scheduling future events.
#include "uart.h"

static const GPIOAddress s_pins[] = {
  { .port = GPIO_PORT_B, .pin = 3 },  // Pin power
  { .port = GPIO_PORT_B, .pin = 4 },  // Pin on_off
};

static const UARTPort s_port = UART_PORT_2;

int main(void) {
  interrupt_init();
  gpio_init();
  soft_timer_init();

  UARTSettings s_settings = {
    .baudrate = 9600,

    .tx = { .port = GPIO_PORT_A, .pin = 2 },
    .rx = { .port = GPIO_PORT_A, .pin = 3 },
    .alt_fn = GPIO_ALTFN_1,
  };

  const GPIOSettings settings_power = {
    .direction = GPIO_DIR_OUT,  // The pin needs to output.
    .state = GPIO_STATE_LOW,    // Start in the "off" state.
    .alt_function = GPIO_ALTFN_NONE,
  };

  const GPIOSettings settings_on_off = {
    .direction = GPIO_DIR_OUT,  // The pin needs to output.
    .state = GPIO_STATE_LOW,    // Start in the "off" state.
    .alt_function = GPIO_ALTFN_NONE,
  };

  evm_gps_settings settings = { .settings_power = &settings_power,
                                .settings_on_off = &settings_on_off,
                                .pin_power = &s_pins[0],
                                .pin_on_off = &s_pins[1],
                                .uart_settings = &s_settings,
                                .port = s_port };
  StatusCode ret = evm_gps_init(&settings);

  soft_timer_start_millis(1000, evm_gps_dump_internal, NULL, NULL);
  return 0;
}
