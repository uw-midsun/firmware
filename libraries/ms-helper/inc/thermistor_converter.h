#pragma once
// Thermistor Reading interface - converts an ADC voltage reading to temperature
// Requires GPIO and Interrupts to be initialized.
#include "adc.h"
#include "gpio.h"
#include "interrupt.h"
#include "soft_timer.h"

#define RESISTANCE_LEN 71

typedef struct {
  uint32_t sibling_resistance; // milliohms
  uint32_t source_voltage; // millivolts
  void *context;

  ADCChannel adc_channel;
} ThermistorStorage;

typedef struct {
  uint32_t sibling_resistance; // milliohms
  uint32_t source_voltage; // millivolts
  void *context;

  GPIOSettings *gpio_settings;
  GPIOAddress *gpio_addr;
  ADCMode adc_mode;
  ADCChannel adc_channel;
} ThermistorSettings;

// Initialize the thermistor interfaces
StatusCode thermistor_converter_init(ThermistorStorage *storage, ThermistorSettings *settings);

// Fetch the temperature reading in milliCelsius
uint32_t thermistor_converter_get_temp(ThermistorStorage *storage);
