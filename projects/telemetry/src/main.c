// Example program for STM32F072 Controller board or Discovery Board.
// Blinks the LEDs sequentially.
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "gpio.h"        // General Purpose I/O control.
#include "soft_timer.h"  // Software timers for scheduling future events.
#include "interrupt.h"   // For enabling interrupts.
#include "gps.h"

// Controller board LEDs
static const GPIOAddress pins[] = {
  {.port = GPIO_PORT_A, .pin = 2 },   //
  {.port = GPIO_PORT_A, .pin = 3 },   //
};

int main(void) {
  // Enable various peripherals
  interrupt_init();
  soft_timer_init();
  gpio_init();

  GPIOSettings settings_tx = {
    .direction = GPIO_DIR_OUT,        // The pin needs to output.
    .state = GPIO_STATE_HIGH,         // Start in the "on" state.
    .alt_function = GPIO_ALTFN_1,  // No connections to peripherals.
    .resistor = GPIO_RES_NONE,        // No need of a resistor to modify floating logic levels.
  };

  GPIOSettings settings_rx = {
    .direction = GPIO_DIR_IN,        // The pin needs to output.
    .state = GPIO_STATE_HIGH,         // Start in the "on" state.
    .alt_function = GPIO_ALTFN_1,  // No connections to peripherals.
    .resistor = GPIO_RES_NONE,        // No need of a resistor to modify floating logic levels.
  };

  gpio_init_pin(&pins[0], &settings_tx);
  gpio_init_pin(&pins[1], &settings_rx);
  
  char test[] = "$GPGGA,053740.000,2503.6319,N,12136.0099,E,1,08,1.1,63.8,M,15.2,M,,0000*64";
  s_nmea_read((uint8_t *) test, sizeof(test)/sizeof(test[0]), test);
  return 0;
}
