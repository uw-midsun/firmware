#include "adc.h"

void adc_init(ADCMode adc_mode) { }

StatusCode adc_set_channel(ADCChannel adc_channel, bool new_state) {
  return STATUS_CODE_UNIMPLEMENTED;
}

StatusCode adc_register_callback(ADCChannel adc_channel, ADCCallback callback, void *context) {
  return STATUS_CODE_UNIMPLEMENTED;
}

StatusCode adc_read_raw(ADCChannel adc_channel, uint16_t *reading) {
  return STATUS_CODE_UNIMPLEMENTED;
}

StatusCode adc_read_converted(ADCChannel adc_channel, uint16_t *reading) {
  return STATUS_CODE_UNIMPLEMENTED;
}
