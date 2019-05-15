#pragma once
// Potentiometer module for LED brightness control written for the Tutorial board
// This module is used as an introduction to Midnight Sun style firmware.
// It is organized in such a way as to expose the user to reading analog data
// Requires soft timers and ADC to be initialized in SINGLE_SHOT mode
#include "adc.h"

// Potentiometer settings structure
typedef struct PotentiometerSettings {
  GpioAddress adc_address;    // Address of the ADC pin
  GpioSettings adc_settings;  // GPIO ADC settings
  uint32_t update_period_s;   // Update period of the ADC
} PotentiometerSettings;

// Potentiometer storage structure
typedef struct PotentiometerStorage {
  AdcChannel adc_channel;    // ADC Channel to read data
  uint32_t update_period_s;  // Update period of the ADC
} PotentiometerStorage;

// Initializes the ADC and soft timers for periodically reading analog data through an ADC
StatusCode potentiometer_init(const PotentiometerSettings *settings, PotentiometerStorage *storage);
