#pragma once
// Thermistor Reading interface for NXRT15XH103FA1B (10k Ohm NTC)
// Requires ADC, GPIO and Interrupts to be initialized.
#include "adc.h"

typedef struct {
  uint32_t sibling_resistance;  // milliohms

  ADCChannel adc_channel;
} ThermistorStorage;

typedef struct {
  uint32_t sibling_resistance;  // milliohms

  ADCChannel adc_channel;
} ThermistorSettings;

// Initialize the thermistor interfaces
StatusCode thermistor_init(ThermistorStorage *storage, ThermistorSettings *settings);

// Fetch the temperature reading in milliCelsius from the MCU's ADC
StatusCode thermistor_get_temp(ThermistorStorage *storage, uint32_t *temperature);

// Calculate the temperature in milliCelsius from milliohms
StatusCode thermistor_calculate_temp(uint16_t resistance, uint32_t *temperature);
