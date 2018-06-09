#pragma once
// Thermistor Reading interface for NXRT15XH103FA1B___
// src:
// https://www.murata.com/en-global/products/productdetail.aspx?cate=luNTCforTempeSenso&partno=NXRT15XH103FA1B___
// Requires ADC, GPIO and Interrupts to be initialized.
#include "adc.h"
#include "gpio.h"
#include "interrupt.h"
#include "soft_timer.h"

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
uint32_t thermistor_get_temp(ThermistorStorage *storage);

// Calculate the temperature based on other data
uint32_t thermistor_calculate_temp(uint16_t resistance);
