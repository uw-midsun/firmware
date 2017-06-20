#pragma once

// Generic ADC Driver

#include <stdbool.h>
#include "gpio.h"
#include "status.h"


typedef enum {
  ADC_MODE_SINGLE = 0,
  ADC_MODE_CONTINUOUS,
} ADCMode;

typedef enum {
  ADC_SAMPLE_RATE_1 = 0,
  ADC_SAMPLE_RATE_2,
  ADC_SAMPLE_RATE_3,
  ADC_SAMPLE_RATE_4,
  ADC_SAMPLE_RATE_5,
  ADC_SAMPLE_RATE_6,
  ADC_SAMPLE_RATE_7,
  ADC_SAMPLE_RATE_8
} ADCSampleRate;

// Initialize the onboard ADC in the specified conversion mode
void adc_init(ADCMode adc_mode);

// Sets the continuous mode sample rate for the given pin.
StatusCode adc_init_pin(GPIOAddress *address, ADCSampleRate adc_sample_rate);

// Returns the current ADC signal as a 12-bit integer. Invalid pins will read 0
uint16_t adc_read(GPIOAddress *address, uint16_t max);

// Disable the ADC
void adc_disable();