#include "fault_monitor.h"
#include <string.h>
#include "log.h"
#include "plutus_event.h"

static void prv_extract_cell_result(uint16_t *result_arr, size_t len, void *context) {
  FaultMonitorStorage *storage = context;

  ltc_afe_request_aux_conversion(storage->settings.ltc_afe);

  memcpy(storage->result.cell_voltages, result_arr, sizeof(storage->result.cell_voltages));

  storage->result.total_voltage = 0;
  bool fault = false;
  for (size_t i = 0; i < len; i++) {
    storage->result.total_voltage += result_arr[i];
    if (result_arr[i] < storage->settings.undervoltage ||
        result_arr[i] > storage->settings.overvoltage) {
      storage->cell_faults[i]++;
      if (storage->cell_faults[i] > PLUTUS_CFG_LTC_AFE_MAX_FAULTS) {
        bps_heartbeat_raise_fault(storage->settings.bps_heartbeat,
                                  EE_BPS_HEARTBEAT_FAULT_SOURCE_LTC_AFE_CELL);
        fault = true;
      }
    } else {
      storage->cell_faults[i] = 0;
    }
  }

  if (!fault) {
    bps_heartbeat_clear_fault(storage->settings.bps_heartbeat,
                              EE_BPS_HEARTBEAT_FAULT_SOURCE_LTC_AFE_CELL);
  }
}

static void prv_extract_aux_result(uint16_t *result_arr, size_t len, void *context) {
  FaultMonitorStorage *storage = context;

  ltc_afe_request_cell_conversion(storage->settings.ltc_afe);

  memcpy(storage->result.temp_voltages, result_arr, sizeof(storage->result.temp_voltages));

  // We assume a fixed resistor on the bottom - node voltage increases as temperature increases
  uint16_t threshold = storage->discharge_temp_node_limit;
  if (storage->result.charging) {
    threshold = storage->charge_temp_node_limit;
  }

  bool fault = false;
  for (size_t i = 0; i < len; i++) {
    if (result_arr[i] > threshold) {
      storage->temp_faults[i]++;
      if (storage->temp_faults[i] > PLUTUS_CFG_LTC_AFE_MAX_FAULTS) {
        fault = true;
      }
    } else {
      storage->temp_faults[i] = 0;
    }
  }

  if (fault) {
    bps_heartbeat_raise_fault(storage->settings.bps_heartbeat,
                        EE_BPS_HEARTBEAT_FAULT_SOURCE_LTC_AFE_TEMP);
  } else {
    bps_heartbeat_clear_fault(storage->settings.bps_heartbeat,
                              EE_BPS_HEARTBEAT_FAULT_SOURCE_LTC_AFE_TEMP);
  }
}

static void prv_extract_current(int32_t value, void *context) {
  FaultMonitorStorage *storage = context;

  storage->result.current = value;
  storage->result.charging = storage->result.current < storage->min_charge_current;

  if (storage->result.current > storage->charge_current_limit ||
      storage->result.current < storage->discharge_current_limit) {
    bps_heartbeat_raise_fault(storage->settings.bps_heartbeat,
                              EE_BPS_HEARTBEAT_FAULT_SOURCE_LTC_ADC);
  } else {
    bps_heartbeat_clear_fault(storage->settings.bps_heartbeat,
                              EE_BPS_HEARTBEAT_FAULT_SOURCE_LTC_ADC);
  }
}

static void prv_handle_adc_timeout(void *context) {
  FaultMonitorStorage *storage = context;

  // Invalid current, so set it to something completely wrong.
  storage->result.current = INT32_MAX;
  bps_heartbeat_raise_fault(storage->settings.bps_heartbeat, EE_BPS_HEARTBEAT_FAULT_SOURCE_LTC_ADC);
}

static StatusCode prv_temp_node_voltage(uint16_t temp_dc, uint16_t *node_voltage) {
  // We assume a fixed resistor on the bottom - node voltage increases as temperature increases
  uint16_t thermistor_resistance_ohms = 0;
  status_ok_or_return(thermistor_calculate_resistance(temp_dc, &thermistor_resistance_ohms));
  *node_voltage = PLUTUS_CFG_THERMISTOR_SUPPLY * PLUTUS_CFG_THERMISTOR_FIXED_RESISTOR_OHMS /
                  (PLUTUS_CFG_THERMISTOR_FIXED_RESISTOR_OHMS + thermistor_resistance_ohms);
  return STATUS_CODE_OK;
}

StatusCode fault_monitor_init(FaultMonitorStorage *storage, const FaultMonitorSettings *settings) {
  storage->settings = *settings;
  memset(storage->cell_faults, 0, sizeof(storage->cell_faults));
  memset(storage->temp_faults, 0, sizeof(storage->temp_faults));
  // Convert mA to uA
  storage->charge_current_limit = settings->overcurrent_charge * 1000;
  storage->discharge_current_limit = settings->overcurrent_discharge * -1000;
  storage->min_charge_current = -1 * settings->charge_current_deadzone;
  prv_temp_node_voltage(settings->overtemp_discharge, &storage->discharge_temp_node_limit);
  prv_temp_node_voltage(settings->overtemp_charge, &storage->charge_temp_node_limit);

  current_sense_register_callback(storage->settings.current_sense, prv_extract_current,
                                  prv_handle_adc_timeout, storage);

  ltc_afe_set_result_cbs(storage->settings.ltc_afe, prv_extract_cell_result, prv_extract_aux_result,
                         storage);

  return ltc_afe_request_cell_conversion(storage->settings.ltc_afe);
}

bool fault_monitor_process_event(FaultMonitorStorage *storage, const Event *e) {
  switch (e->id) {
    case PLUTUS_EVENT_AFE_FAULT:
      if (storage->num_afe_fsm_faults++ > PLUTUS_CFG_LTC_AFE_FSM_MAX_FAULTS) {
        LOG_DEBUG("AFE FSM fault %d\n", e->data);
        bps_heartbeat_raise_fault(storage->settings.bps_heartbeat,
                                  EE_BPS_HEARTBEAT_FAULT_SOURCE_LTC_AFE_FSM);
      }

      // Attempt to recover from failure
      ltc_afe_request_cell_conversion(storage->settings.ltc_afe);
      return true;
    case PLUTUS_EVENT_AFE_CALLBACK_RUN:
      storage->num_afe_fsm_faults = 0;
      bps_heartbeat_clear_fault(storage->settings.bps_heartbeat,
                                EE_BPS_HEARTBEAT_FAULT_SOURCE_LTC_AFE_FSM);
      return true;
    default:
      return false;
  }

  return false;
}
