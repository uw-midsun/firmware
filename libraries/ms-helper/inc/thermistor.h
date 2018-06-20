#pragma once
// Thermistor Reading interface for NXRT15XH103FA1B (10k Ohm NTC)
// Requires ADC, GPIO and Interrupts to be initialized.
#include "adc.h"
#include "gpio.h"

typedef struct {
  uint32_t sibling_resistance_ohms;
  bool is_thermistor_first;
  ADCChannel adc_channel;
} ThermistorStorage;

// Initialize the thermistor GPIO pins, and adc channels
StatusCode thermistor_init(ThermistorStorage *storage, GPIOAddress thermistor_gpio_address,
                           uint32_t sibling_resistance_ohms, bool is_thermistor_first);

// Fetch the temperature reading in milliCelsius from the MCU's ADC
StatusCode thermistor_get_temp(ThermistorStorage *storage, uint32_t *temperature_millicelcius);

// Calculate the temperature in milliCelsius from milliohms
StatusCode thermistor_calculate_temp(uint32_t thermistor_resistance_milliohms,
                                     uint32_t *temperature_millicelcius);
