#include "fault_monitor.h"
#include <string.h>
#include "log.h"
#include "plutus_event.h"
#include "thermistor.h"

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
      fault = true;
    }
  }

  if (fault) {
    bps_heartbeat_raise_fault(storage->settings.bps_heartbeat,
                              EE_BPS_HEARTBEAT_FAULT_SOURCE_LTC_AFE);
  } else {
    bps_heartbeat_clear_fault(storage->settings.bps_heartbeat,
                              EE_BPS_HEARTBEAT_FAULT_SOURCE_LTC_AFE);
  }
}

static void prv_extract_aux_result(uint16_t *result_arr, size_t len, void *context) {
  FaultMonitorStorage *storage = context;

  ltc_afe_request_cell_conversion(storage->settings.ltc_afe);

  memcpy(storage->result.temp_voltages, result_arr, sizeof(storage->result.temp_voltages));

  bool fault = false;
  for (size_t i = 0; i < len; i++) {
    if (storage->result.charging) {
      if (result_arr[i] > storage->charge_voltage_limit) {
        fault = true;
      }
    } else {
      if (result_arr[i] > storage->discharge_voltage_limit) {
        fault = true;
      }
    }
  }

  if (fault) {
    bps_heartbeat_raise_fault(storage->settings.bps_heartbeat,
                              EE_BPS_HEARTBEAT_FAULT_SOURCE_LTC_AFE);
  } else {
    bps_heartbeat_clear_fault(storage->settings.bps_heartbeat,
                              EE_BPS_HEARTBEAT_FAULT_SOURCE_LTC_AFE);
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

  bps_heartbeat_raise_fault(storage->settings.bps_heartbeat, EE_BPS_HEARTBEAT_FAULT_SOURCE_LTC_ADC);
}

StatusCode thermistor_temperature_to_voltage(uint16_t temperature_dc, uint32_t supply_voltage,
                                             uint16_t *node_voltage) {
  uint16_t thermistor_resistance_ohms = 0;
  thermistor_calculate_resistance(temperature_dc, &thermistor_resistance_ohms);
  *node_voltage = (uint16_t)(supply_voltage) * (thermistor_resistance_ohms) /
                  (THERMISTOR_FIXED_RESISTANCE_OHMS + thermistor_resistance_ohms);
  return STATUS_CODE_OK;
}

StatusCode fault_monitor_init(FaultMonitorStorage *storage, const FaultMonitorSettings *settings) {
  storage->settings = *settings;
  storage->num_afe_faults = 0;
  // Convert mA to uA
  storage->charge_current_limit = settings->overcurrent_charge * 1000;
  storage->discharge_current_limit = settings->overcurrent_discharge * -1000;
  storage->min_charge_current = -1 * settings->charge_current_deadzone;
  thermistor_temperature_to_voltage(settings->overtemp_discharge, settings->overvoltage,
                                    &(storage->discharge_voltage_limit));
  thermistor_temperature_to_voltage(settings->overtemp_charge, settings->overvoltage,
                                    &(storage->charge_voltage_limit));

  current_sense_register_callback(storage->settings.current_sense, prv_extract_current,
                                  prv_handle_adc_timeout, storage);

  ltc_afe_set_result_cbs(storage->settings.ltc_afe, prv_extract_cell_result, prv_extract_aux_result,
                         storage);

  return ltc_afe_request_cell_conversion(storage->settings.ltc_afe);
}

bool fault_monitor_process_event(FaultMonitorStorage *storage, const Event *e) {
  switch (e->id) {
    case PLUTUS_EVENT_AFE_FAULT:
      if (storage->num_afe_faults++ > PLUTUS_CFG_LTC_AFE_FSM_MAX_FAULTS) {
        LOG_DEBUG("AFE FSM fault %d\n", e->data);
        bps_heartbeat_raise_fault(storage->settings.bps_heartbeat,
                                  EE_BPS_HEARTBEAT_FAULT_SOURCE_LTC_AFE_FSM);
      }

      // Attempt to recover from failure
      ltc_afe_request_cell_conversion(storage->settings.ltc_afe);
      return true;
    case PLUTUS_EVENT_AFE_CALLBACK_RUN:
      storage->num_afe_faults = 0;
      bps_heartbeat_clear_fault(storage->settings.bps_heartbeat,
                                EE_BPS_HEARTBEAT_FAULT_SOURCE_LTC_AFE_FSM);
      return true;
    default:
      return false;
  }

  return false;
}
