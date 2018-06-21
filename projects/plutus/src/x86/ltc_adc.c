#include "ltc_adc.h"

// x86 module for mocking current sense data

#include "critical_section.h"
#include "ltc2484.h"

static int32_t s_test_voltage = 0;

static void prv_ltc_adc_read(SoftTimerID timer_id, void *context) {
  LtcAdcStorage *storage = (LtcAdcStorage *)context;

  storage->buffer.value = s_test_voltage;

  if (storage->callback != NULL) {
    storage->callback(&storage->buffer.value, storage->context);
  }

  soft_timer_start_millis(LTC2484_MAX_CONVERSION_TIME_MS, prv_ltc_adc_read, storage,
                          &storage->buffer.timer_id);
}

StatusCode ltc_adc_init(LtcAdcStorage *storage, const LtcAdcSettings *settings) {
  if (storage == NULL || settings->filter_mode >= NUM_LTC_ADC_FILTER_MODES) {
    return status_code(STATUS_CODE_INVALID_ARGS);
  }

  // Initialize test voltage to zero
  s_test_voltage = 0;

  return soft_timer_start_millis(LTC2484_MAX_CONVERSION_TIME_MS, prv_ltc_adc_read, storage,
                                 &storage->buffer.timer_id);
}

StatusCode ltc_adc_register_callback(LtcAdcStorage *storage, LtcAdcCallback callback,
                                     void *context) {
  if (storage == NULL) {
    return status_code(STATUS_CODE_INVALID_ARGS);
  }

  bool disabled = critical_section_start();
  storage->callback = callback;
  storage->context = context;
  critical_section_end(disabled);

  return STATUS_CODE_OK;
}

StatusCode test_ltc_adc_set_input_voltage(int32_t input_voltage) {
  s_test_voltage = input_voltage;

  return STATUS_CODE_OK;
}
