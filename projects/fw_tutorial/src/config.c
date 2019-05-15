#include "config.h"

// Button settings
static ButtonSettings button_settings = {
  .button_addresses = { [BUTTON_COLOUR_GREEN] = { GPIO_PORT_B, 0 },     // Green button pin
                        [BUTTON_COLOUR_YELLOW] = { GPIO_PORT_A, 7 } },  // Yellow button pin
  .led_addresses = { [BUTTON_COLOUR_GREEN] = { GPIO_PORT_B, 3 },        // Green button LED pin
                     [BUTTON_COLOUR_YELLOW] = { GPIO_PORT_B, 4 } },     // Yellow button LED pin
  .gpio_settings = { .direction = GPIO_DIR_IN,
                     .state = GPIO_STATE_LOW,
                     .resistor = GPIO_RES_NONE,
                     .alt_function = GPIO_ALTFN_NONE },
  .led_settings = { .direction = GPIO_DIR_OUT,
                    .state = GPIO_STATE_LOW,
                    .resistor = GPIO_RES_NONE,
                    .alt_function = GPIO_ALTFN_NONE },
  .interrupt_settings =
      {
          .type = INTERRUPT_TYPE_INTERRUPT,
          .priority = INTERRUPT_PRIORITY_NORMAL  // No other interrupts, leave as normal
      },
  .interrupt_edge = INTERRUPT_EDGE_RISING  // Trigger on the rising edge
};

// Potentiometer settings
static PotentiometerSettings potentiometer_settings = {
  // TODO(ELEC-624): Change this once hardware revision updates
  .adc_address = { GPIO_PORT_A, 0 },  // Potentiometer pin
  .adc_settings =
      {
          .direction = GPIO_DIR_IN,
          .state = GPIO_STATE_LOW,
          .resistor = GPIO_RES_NONE,
          .alt_function = GPIO_ALTFN_ANALOG  // Alternate function as ADC
      },
  .update_period_s = CONFIG_ADC_UPDATE_PERIOD_S  // Update period
};

ButtonSettings *config_load_buttons(void) {
  return &button_settings;
}

PotentiometerSettings *config_load_potentiometer(void) {
  return &potentiometer_settings;
}
