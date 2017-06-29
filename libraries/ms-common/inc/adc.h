#pragma once

// Analog to Digital Converter HAL Inteface

#include <stdbool.h>
#include <stdint.h>

#include "status.h"

typedef enum {
  ADC_MODE_SINGLE = 0,
  ADC_MODE_CONTINUOUS,
} ADCMode;

typedef enum {
  ADC_CHANNEL_0 = 0,
  ADC_CHANNEL_1,
  ADC_CHANNEL_2,
  ADC_CHANNEL_3,
  ADC_CHANNEL_4,
  ADC_CHANNEL_5,
  ADC_CHANNEL_6,
  ADC_CHANNEL_7,
  ADC_CHANNEL_8,
  ADC_CHANNEL_9,
  ADC_CHANNEL_10,
  ADC_CHANNEL_11,
  ADC_CHANNEL_12,
  ADC_CHANNEL_13,
  ADC_CHANNEL_14,
  ADC_CHANNEL_15,
  ADC_CHANNEL_TEMP,
  ADC_CHANNEL_REF,
  ADC_CHANNEL_BAT,
  NUM_ADC_CHANNEL
} ADCChannel;

typedef void (*ADCCallback)(ADCChannel adc_channel, uint16_t reading, void *context);

// Initialize the ADC to the desired conversion mode
void adc_init(ADCMode adc_mode);

// Enable or disable a given channel
StatusCode adc_set_channel(ADCChannel adc_channel, bool new_state);

// Register a callback function to be called when the specified channel completes a conversion
StatusCode adc_register_callback(ADCChannel adc_channel, ADCCallback callback, void *context);

// Obtain the raw 12-bit value read by the specified channel
StatusCode adc_read_value(ADCChannel adc_channel, uint16_t *reading);
