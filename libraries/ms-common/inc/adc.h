#pragma once  

// Generic ADC Driver

#include <stdbool.h>
#include <stdint.h>

#include "status.h"

typedef uint8_t ADCChannel;

typedef void (*adc_callback)(ADCChannel adc_channel, uint16_t reading, void *context);

typedef enum {
  ADC_MODE_SINGLE = 0,
  ADC_MODE_CONTINUOUS,
} ADCMode;

// Initialize the ADC
void adc_init(ADCMode adc_mode);

// Select or deselect the channel for conversions
void adc_set_channel(ADCChannel adc_channel, bool new_state);

// Register a callback function for an ADC Channel
StatusCode adc_interrupt_callback(ADCChannel adc_channel, adc_callback callback, void *context);

// Obtain the current value for the given channel
uint16_t adc_read(ADCChannel adc_channel);

// Disable the ADC
void adc_disable();
