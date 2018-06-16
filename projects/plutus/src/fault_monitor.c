#include "fault_monitor.h"
#include <string.h>
#include "log.h"

static void prv_extract_cell_result(uint16_t *result_arr, size_t len, void *context) {
  FaultMonitorStorage *storage = context;

  ltc_afe_request_aux_conversion(storage->settings.ltc_afe);

  memcpy(storage->result.cell_voltages, result_arr, sizeof(storage->result.cell_voltages));

  for (size_t i = 0; i < len; i++) {
    if (result_arr[i] < storage->settings.undervoltage ||
        result_arr[i] > storage->settings.overvoltage) {
      bps_heartbeat_raise_fault(storage->settings.bps_heartbeat,
                                EE_BPS_HEARTBEAT_FAULT_SOURCE_LTC_AFE);
      return;
    }
  }

  bps_heartbeat_clear_fault(storage->settings.bps_heartbeat, EE_BPS_HEARTBEAT_FAULT_SOURCE_LTC_AFE);
}

static void prv_extract_aux_result(uint16_t *result_arr, size_t len, void *context) {
  FaultMonitorStorage *storage = context;

  ltc_afe_request_cell_conversion(storage->settings.ltc_afe);

  memcpy(storage->result.temp_voltages, result_arr, sizeof(storage->result.temp_voltages));

  // TODO(ELEC-439): Add temp faulting
  // for (size_t i = 0; i < len; i++) {
  //   bps_heartbeat_raise_fault(storage->settings.bps_heartbeat,
  //   BPS_HEARTBEAT_FAULT_SOURCE_LTC_ADC);
  // }
}

static void prv_extract_current(int32_t *value, void *context) {
  FaultMonitorStorage *storage = context;

  storage->result.current = *value;
}

StatusCode fault_monitor_init(FaultMonitorStorage *storage, const FaultMonitorSettings *settings) {
  storage->settings = *settings;

  ltc_adc_register_callback(storage->settings.ltc_adc, prv_extract_current, storage);

  ltc_afe_set_result_cbs(storage->settings.ltc_afe, prv_extract_cell_result, prv_extract_aux_result,
                         storage);

  return ltc_afe_request_cell_conversion(storage->settings.ltc_afe);
}
