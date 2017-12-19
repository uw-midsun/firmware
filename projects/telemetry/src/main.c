#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "gpio.h"  // General Purpose I/O control.
#include "gps.h"
#include "interrupt.h"   // For enabling interrupts.
#include "soft_timer.h"  // Software timers for scheduling future events.

// As far as I can tell we just made up the values below
static const GPIOAddress pins[] = {
  { .port = GPIO_PORT_A, .pin = 2 },  //
  { .port = GPIO_PORT_A, .pin = 3 },  //
  { .port = GPIO_PORT_A, .pin = 4 },  //
};

static const UARTSettings s_settings = {
  .baudrate = 9600,

  .tx = { .port = GPIO_PORT_A, .pin = 2 },
  .rx = { .port = GPIO_PORT_A, .pin = 3 },
  .alt_fn = GPIO_ALTFN_1,
};

void gga_handler(GGASentence result) {
  return;
}

int main(void) {
  // Enable various peripherals
  interrupt_init();
  soft_timer_init();
  gpio_init();

  const GPIOSettings settings_tx = {
    .direction = GPIO_DIR_OUT,     // The pin needs to output.
    .state = GPIO_STATE_HIGH,      // Start in the "on" state.
    .alt_function = GPIO_ALTFN_1,  // No connections to peripherals.
    .resistor = GPIO_RES_NONE,     // No need of a resistor to modify floating
                                   // logic levels.
  };

  const GPIOSettings settings_rx = {
    .direction = GPIO_DIR_IN,      // The pin needs to input.
    .state = GPIO_STATE_HIGH,      // Start in the "on" state.
    .alt_function = GPIO_ALTFN_1,  // No connections to peripherals.
    .resistor = GPIO_RES_NONE,     // No need of a resistor to modify floating
                                   // logic levels.
  };

  const GPIOSettings settings_power = {
    .direction = GPIO_DIR_OUT,     // The pin needs to iout.
    .state = GPIO_STATE_HIGH,      // Start in the "on" state.
    .alt_function = GPIO_ALTFN_1,  // No connections to peripherals.
    .resistor = GPIO_RES_NONE,     // No need of a resistor to modify floating
                                   // logic levels.
  };

  EvmSettings settings = { .settings_tx = &settings_tx,
                           .settings_rx = &settings_rx,
                           .settings_power = &settings_power,
                           .pin_tx = &pins[0],
                           .pin_rx = &pins[1],
                           .pin_power = &pins[2],
                           .uart_settings = &s_settings };
  evm_gps_init(&settings);
  add_gga_handler(gga_handler, NULL);

  return 0;
}
