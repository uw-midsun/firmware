#include "adc.h"

void adc_init(ADCMode adc_mode) { }

StatusCode adc_set_channel(ADCChannel adc_channel, bool new_state) { }

StatusCode adc_register_callback(ADCChannel adc_channel, ADCCallback callback, void *context) { }

StatusCode adc_read_value(ADCChannel adc_channel, uint16_t *reading) { }
