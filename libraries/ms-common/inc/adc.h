#pragma once
// Analog to Digital Converter HAL Inteface
// Requires GPIO and interrupts to be initialized.
#include <stdbool.h>
#include <stdint.h>

#include "gpio.h"
#include "status.h"

typedef enum {
  ADC_MODE_SINGLE = 0,
  ADC_MODE_CONTINUOUS,
  NUM_ADC_MODES,
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
  NUM_ADC_CHANNELS,
} ADCChannel;

typedef void (*ADCCallback)(ADCChannel adc_channel, void *context);

// Initialize the ADC to the desired conversion mode
void adc_init(ADCMode adc_mode);

// Enable or disable a given channel.
// A race condition may occur when setting a channel during a conversion.
// However, it should not
// cause issues given the intended use cases
StatusCode adc_set_channel(ADCChannel adc_channel, bool new_state);

// Return a channel corresponding to the given GPIO address
StatusCode adc_get_channel(GPIOAddress address, ADCChannel *adc_channel);

// Register a callback function to be called when the specified channel
// completes a conversion
StatusCode adc_register_callback(ADCChannel adc_channel, ADCCallback callback, void *context);

// Obtain the raw 12-bit value read by the specified channel
StatusCode adc_read_raw(ADCChannel adc_channel, uint16_t *reading);

// Obtain the converted value at the specified channel
StatusCode adc_read_converted(ADCChannel adc_channel, uint16_t *reading);
