#include "fault_monitor.h"
#include "can_transmit.h"
#include "log.h"

StatusCode fault_monitor_check(FaultMonitorSettings *settings) {
  bool is_charging = false;

  uint16_t voltages[PLUTUS_CFG_AFE_TOTAL_CELLS] = { 0 };
  StatusCode status = ltc_afe_read_all_voltage(settings->ltc_afe, voltages, PLUTUS_CFG_AFE_TOTAL_CELLS);
  if (status != STATUS_CODE_OK) {
    LOG_DEBUG("Invalid status %d\n", status);
    bps_heartbeat_raise_fault(settings->bps_heartbeat, BPS_HEARTBEAT_FAULT_SOURCE_LTC_AFE);
  }
  bps_heartbeat_clear_fault(settings->bps_heartbeat, BPS_HEARTBEAT_FAULT_SOURCE_LTC_AFE);

  uint16_t aux_voltages[PLUTUS_CFG_AFE_TOTAL_CELLS] = { 0 };
  status = ltc_afe_read_all_aux(settings->ltc_afe, aux_voltages, PLUTUS_CFG_AFE_TOTAL_CELLS);
  if (status != STATUS_CODE_OK) {
    LOG_DEBUG("Invalid status (aux) %d\n", status);
    bps_heartbeat_raise_fault(settings->bps_heartbeat, BPS_HEARTBEAT_FAULT_SOURCE_LTC_AFE);
  }
  bps_heartbeat_clear_fault(settings->bps_heartbeat, BPS_HEARTBEAT_FAULT_SOURCE_LTC_AFE);

  bool had_fault = false;
  for (size_t i = 0; i < PLUTUS_CFG_AFE_TOTAL_CELLS; i++) {
    // TODO: add temperature check
    had_fault |= (voltages[i] < settings->undervoltage || voltages[i] > settings->overvoltage);

    CAN_TRANSMIT_BATTERY_VCT(i, voltages[i], 0, 0);

    LOG_DEBUG("C%d: cell %d.%dmV, aux %d.%dmV\n", i, voltages[i] / 10, voltages[i] % 10,
              aux_voltages[i] / 10, aux_voltages[i] % 10);
  }

  if (had_fault) {
    bps_heartbeat_raise_fault(settings->bps_heartbeat, BPS_HEARTBEAT_FAULT_SOURCE_LTC_AFE);
  } else {
    bps_heartbeat_clear_fault(settings->bps_heartbeat, BPS_HEARTBEAT_FAULT_SOURCE_LTC_AFE);
  }

  return STATUS_CODE_OK;
}
