#include <stddef.h>

#include "delay.h"
#include "log.h"
#include "plutus_cfg.h"

#include "fault_monitor.h"
#include "plutus_sys.h"

static PlutusSysStorage s_plutus;

int main(void) {
  plutus_sys_init(&s_plutus, PLUTUS_SYS_TYPE_MASTER);

  // TODO(ELEC-439): fix this
  FaultMonitorSettings fault_monitor = {
    .bps_heartbeat = &s_plutus.bps_heartbeat,
    .ltc_afe = &s_plutus.ltc_afe,

    .overvoltage = PLUTUS_CFG_CELL_OVERVOLTAGE,
    .undervoltage = PLUTUS_CFG_CELL_UNDERVOLTAGE,
  };

  Event e = { 0 };
  while (true) {
    if (status_ok(event_process(&e))) {
      fsm_process_event(CAN_FSM, &e);
    }

    // TODO(ELEC-439): Only check for faults if master?
    fault_monitor_check(&fault_monitor);
  }
}
