#include <stddef.h>

#include "delay.h"
#include "log.h"
#include "plutus_cfg.h"

#include "can_transmit.h"
#include "fault_monitor.h"
#include "plutus_sys.h"
#include "wait.h"

static PlutusSysStorage s_plutus;
static FaultMonitorStorage s_fault_monitor;

static size_t i = 0;

static void prv_periodic_tx_debug(SoftTimerID timer_id, void *context) {
  FaultMonitorResult *result = &s_fault_monitor.result;

  if (i < PLUTUS_CFG_AFE_TOTAL_CELLS) {
    CAN_TRANSMIT_BATTERY_VT(i, result->cell_voltages[i], result->temp_voltages[i]);
    i++;
  } else if (i == PLUTUS_CFG_AFE_TOTAL_CELLS) {
    CAN_TRANSMIT_BATTERY_CURRENT((uint32_t)result->current);
    i = 0;
  }

  soft_timer_start_millis(PLUTUS_CFG_TELEMETRY_PERIOD_MS, prv_periodic_tx_debug, NULL, NULL);
}

int main(void) {
  PlutusSysType board_type = plutus_sys_get_type();
  plutus_sys_init(&s_plutus, board_type);
  LOG_DEBUG("Board type: %d\n", board_type);

  if (board_type == PLUTUS_SYS_TYPE_MASTER) {
    const FaultMonitorSettings fault_settings = {
      .bps_heartbeat = &s_plutus.bps_heartbeat,
      .ltc_afe = &s_plutus.ltc_afe,
      .ltc_adc = &s_plutus.ltc_adc,

      .overvoltage = PLUTUS_CFG_CELL_OVERVOLTAGE,
      .undervoltage = PLUTUS_CFG_CELL_UNDERVOLTAGE,
    };

    fault_monitor_init(&s_fault_monitor, &fault_settings);
    soft_timer_start_millis(PLUTUS_CFG_TELEMETRY_PERIOD_MS, prv_periodic_tx_debug, NULL, NULL);
  }

  Event e = { 0 };
  while (true) {
    wait();
    while (status_ok(event_process(&e))) {
      fsm_process_event(CAN_FSM, &e);
      ltc_afe_process_event(&s_plutus.ltc_afe, &e);
    }
  }
}
