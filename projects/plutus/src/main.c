#include <stddef.h>

#include "delay.h"
#include "log.h"
#include "plutus_cfg.h"

#include "can_transmit.h"
#include "fault_monitor.h"
#include "plutus_event.h"
#include "plutus_sys.h"
#include "wait.h"

static PlutusSysStorage s_plutus;
static FaultMonitorStorage s_fault_monitor;

static size_t s_telemetry_counter = 0;

static void prv_periodic_tx_debug(SoftTimerId timer_id, void *context) {
  FaultMonitorResult *result = &s_fault_monitor.result;
  if (s_telemetry_counter < PLUTUS_CFG_AFE_TOTAL_CELLS) {
    CAN_TRANSMIT_BATTERY_VT(s_telemetry_counter, result->cell_voltages[s_telemetry_counter],
                            result->temp_voltages[s_telemetry_counter]);
    s_telemetry_counter++;
  } else if (s_telemetry_counter == PLUTUS_CFG_AFE_TOTAL_CELLS) {
    CAN_TRANSMIT_BATTERY_AGGREGATE_VC(result->total_voltage, (uint32_t)result->current);
    s_telemetry_counter = 0;
  }

  soft_timer_start_millis(PLUTUS_CFG_TELEMETRY_PERIOD_MS, prv_periodic_tx_debug, NULL, NULL);
}

int main(void) {
  PlutusSysType board_type = plutus_sys_get_type();
  plutus_sys_init(&s_plutus, board_type);
  LOG_DEBUG("Board type: %d\n", board_type);
    const FaultMonitorSettings fault_settings = {
      .bps_heartbeat = &s_plutus.bps_heartbeat,
      .ltc_afe = &s_plutus.ltc_afe,
      .current_sense = &s_plutus.current_sense,

      .overvoltage = PLUTUS_CFG_CELL_OVERVOLTAGE,
      .undervoltage = PLUTUS_CFG_CELL_UNDERVOLTAGE,

      .overcurrent_charge = PLUTUS_CFG_OVERCURRENT_DISCHARGE,
      .overcurrent_discharge = PLUTUS_CFG_OVERCURRENT_CHARGE,

      .overtemp_charge = PLUTUS_CFG_OVERTEMP_CHARGE,
      .overtemp_discharge = PLUTUS_CFG_OVERTEMP_DISCHARGE,
    };
  if (board_type == PLUTUS_SYS_TYPE_MASTER) {\
    fault_monitor_init(&s_fault_monitor, &fault_settings);
    soft_timer_start_millis(PLUTUS_CFG_TELEMETRY_PERIOD_MS, prv_periodic_tx_debug, NULL, NULL);
  }

  current_sense_zero_reset(&s_plutus.current_sense);

  Event e = { 0 };
  GpioAddress killswitch_monitor = PLUTUS_CFG_KILLSWITCH_MONITOR;
  GpioState kill_sw_monitor_state; 

  while (true) {
    wait();
    while (status_ok(event_process(&e))) {
      can_process_event(&e);
      if (board_type == PLUTUS_SYS_TYPE_MASTER) {
        fault_monitor_process_event(&s_fault_monitor, &e);
        ltc_afe_process_event(&s_plutus.ltc_afe, &e);
        gpio_get_state(&killswitch_monitor, &kill_sw_monitor_state);
        if (kill_sw_monitor_state == GPIO_STATE_LOW) {
          bps_heartbeat_raise_fault(s_fault_monitor.settings.bps_heartbeat, EE_BPS_HEARTBEAT_FAULT_SOURCE_KILLSWITCH);
        }
      }
    }
  }
}
