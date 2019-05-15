#include "config.h"

// Button settings
static ButtonSettings button_settings = {
  // Populate button settings with appropriate configurations
  // Enter your code below ...
};

// Potentiometer settings
static PotentiometerSettings potentiometer_settings = {
  // Populate LED settings with appropriate configurations
  // Enter your code below ...
};

ButtonSettings *config_load_buttons(void) {
  return &button_settings;
}

PotentiometerSettings *config_load_potentiometer(void) {
  return &potentiometer_settings;
}
