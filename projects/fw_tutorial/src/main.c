// Standard library includes
#include <stdbool.h>

// Midnight Sun common includes
#include "interrupt.h"
#include "log.h"
#include "soft_timer.h"
#include "wait.h"

// Module specific includes
#include "button.h"
#include "config.h"
#include "potentiometer.h"

// Button and potentiometer storage which will persist throughout the duration of the program
static ButtonStorage button_storage[NUM_BUTTON_COLOURS];
static PotentiometerStorage potentiometer_storage;

int main(void) {
  // Initialize
  gpio_init();
  interrupt_init();
  soft_timer_init();
  gpio_it_init();
  adc_init(ADC_MODE_SINGLE);

  // Initialize the button module
  // This will expose push-button control of the on-board LEDs
  button_init(config_load_buttons(), button_storage);

  // Initialize the potentiometer module
  // This will enable reading of analog data through the ADCs
  potentiometer_init(config_load_potentiometer(), &potentiometer_storage);

  // Superloop!
  while (true) {
    // Do stuff here!
  }
}
