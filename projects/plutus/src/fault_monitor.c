#include "fault_monitor.h"
#include "log.h"

StatusCode fault_monitor_check(const FaultMonitorSettings *settings, FaultMonitorResult *result) {
  bool is_charging = false;

  StatusCode status = ltc_afe_read_all_voltage(settings->ltc_afe, result->cell_voltages,
                                               PLUTUS_CFG_AFE_TOTAL_CELLS);
  if (status != STATUS_CODE_OK) {
    LOG_DEBUG("Invalid status %d\n", status);
    bps_heartbeat_raise_fault(settings->bps_heartbeat, BPS_HEARTBEAT_FAULT_SOURCE_LTC_AFE);
  }
  bps_heartbeat_clear_fault(settings->bps_heartbeat, BPS_HEARTBEAT_FAULT_SOURCE_LTC_AFE);

  status =
      ltc_afe_read_all_aux(settings->ltc_afe, result->temp_voltages, PLUTUS_CFG_AFE_TOTAL_CELLS);
  if (status != STATUS_CODE_OK) {
    LOG_DEBUG("Invalid status (aux) %d\n", status);
    bps_heartbeat_raise_fault(settings->bps_heartbeat, BPS_HEARTBEAT_FAULT_SOURCE_LTC_AFE);
  }
  bps_heartbeat_clear_fault(settings->bps_heartbeat, BPS_HEARTBEAT_FAULT_SOURCE_LTC_AFE);

  bool had_fault = false;
  for (size_t i = 0; i < PLUTUS_CFG_AFE_TOTAL_CELLS; i++) {
    // TODO(ELEC-439): add temperature check
    had_fault |= (result->cell_voltages[i] < settings->undervoltage ||
                  result->cell_voltages[i] > settings->overvoltage);
  }

  if (had_fault) {
    bps_heartbeat_raise_fault(settings->bps_heartbeat, BPS_HEARTBEAT_FAULT_SOURCE_LTC_AFE);
  } else {
    bps_heartbeat_clear_fault(settings->bps_heartbeat, BPS_HEARTBEAT_FAULT_SOURCE_LTC_AFE);
  }

  return STATUS_CODE_OK;
}
