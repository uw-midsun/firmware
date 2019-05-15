#include "config.h"

// Button settings
static ButtonSettings button_settings = {
  .button_addresses = { [BUTTON_COLOUR_GREEN] = { GPIO_PORT_B, 0 },     // Green button pin
                        [BUTTON_COLOUR_YELLOW] = { GPIO_PORT_A, 7 } },  // Yellow button pin
  .led_addresses = { [BUTTON_COLOUR_GREEN] = { GPIO_PORT_B, 3 },        // Green button LED pin
                     [BUTTON_COLOUR_YELLOW] = { GPIO_PORT_B, 4 } },     // Yellow button LED pin
  // No other interrupts expected, leave priority as normal
  .interrupt_settings = { INTERRUPT_TYPE_INTERRUPT, INTERRUPT_PRIORITY_NORMAL },
  .interrupt_edge = INTERRUPT_EDGE_RISING  // Trigger on the rising edge
};

// Potentiometer settings
static PotentiometerSettings potentiometer_settings = {
  // TODO(ELEC-624): Change this once hardware revision updates
  .adc_address = { GPIO_PORT_A, 0 },             // Potentiometer pin
  .update_period_s = CONFIG_ADC_UPDATE_PERIOD_S  // Update period
};

ButtonSettings *config_load_buttons(void) {
  return &button_settings;
}

PotentiometerSettings *config_load_potentiometer(void) {
  return &potentiometer_settings;
}
