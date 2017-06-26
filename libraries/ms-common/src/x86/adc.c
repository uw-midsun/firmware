#include "adc.h"

void adc_init(ADCMode adc_mode) { }

StatusCode adc_set_channel(ADCChannel adc_channel, bool new_state) { }

StatusCode adc_register_callback(ADCChannel adc_channel, ADCCallback callback, void *context) { }

void adc_start_continuous() { }

uint16_t adc_read_value(ADCChannel adc_channel) { }

void adc_disable() { }
