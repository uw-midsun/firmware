// temporary header file - we will improve this
#pragma once

#include "adc.h"
#include "gpio.h"
#include "interrupt.h"
#include "soft_timer.h"

typedef struct ThermistorStorage {
  uint32_t sibling_resistance;
  uint32_t source_voltage;
  ADCChannel channel;
  // GPIOAddress address;
  // GPIOSettings settings; 
} ThermistorStorage;
// initialize the termistor

// StatusCode thermistor_converter_init(ThermistorStorage *thermistor, uint32_t sibling_resistance, uint32_t source_voltage);

// get the temperature
uint32_t thermistor_converter_get_temp(ThermistorStorage *thermistor);