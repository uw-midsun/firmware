#include <stddef.h>

#include "delay.h"
#include "log.h"
#include "plutus_cfg.h"

#include "can_transmit.h"
#include "fault_monitor.h"
#include "plutus_sys.h"

static PlutusSysStorage s_plutus;
static FaultMonitorResult s_result;

static void prv_periodic_tx_debug(SoftTimerID timer_id, void *context) {
  // TODO(ELEC-439): Output current data
  // CAN_TRANSMIT_BATTERY_CURRENT();
  for (size_t i = 0; i < PLUTUS_CFG_AFE_TOTAL_CELLS; i++) {
    LOG_DEBUG("C%d %d.%dmV, aux %d.%dmV\n", i, s_result.cell_voltages[i] / 10,
              s_result.cell_voltages[i] % 10, s_result.temp_voltages[i] / 10,
              s_result.temp_voltages[i] % 10);
    CAN_TRANSMIT_BATTERY_VT(i, s_result.cell_voltages[i], s_result.temp_voltages[i]);
  }

  soft_timer_start_millis(PLUTUS_CFG_TELEMETRY_PERIOD_MS, prv_periodic_tx_debug, NULL, NULL);
}

int main(void) {
  PlutusSysType board_type = plutus_sys_get_type();
  plutus_sys_init(&s_plutus, board_type);
  LOG_DEBUG("Board type: %d\n", board_type);

  FaultMonitorSettings fault_monitor = {
    .bps_heartbeat = &s_plutus.bps_heartbeat,
    .ltc_afe = &s_plutus.ltc_afe,

    .overvoltage = PLUTUS_CFG_CELL_OVERVOLTAGE,
    .undervoltage = PLUTUS_CFG_CELL_UNDERVOLTAGE,
  };

  if (board_type == PLUTUS_SYS_TYPE_MASTER) {
    // Only bother dumping telemetry data if master
    soft_timer_start_millis(PLUTUS_CFG_TELEMETRY_PERIOD_MS, prv_periodic_tx_debug, NULL, NULL);
  }

  Event e = { 0 };
  while (true) {
    if (status_ok(event_process(&e))) {
      fsm_process_event(CAN_FSM, &e);
    }

    // if (board_type == PLUTUS_SYS_TYPE_MASTER) {
    //   // Only check for faults if master
    //   fault_monitor_check(&fault_monitor, &s_result);
    // }
  }
}
