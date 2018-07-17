#include <stddef.h>

#include "delay.h"
#include "log.h"
#include "plutus_cfg.h"

#include "can_transmit.h"
#include "fault_monitor.h"
#include "plutus_event.h"
#include "plutus_sys.h"
#include "wait.h"

#define PLUTUS_MAX_DIFFERENCE_MILLIVOLTS 150

static PlutusSysStorage s_plutus;
static FaultMonitorStorage s_fault_monitor;

static size_t s_telemetry_counter = 0;
int main(void) {
  PlutusSysType board_type = plutus_sys_get_type();
  plutus_sys_init(&s_plutus, board_type);

  Event e = { 0 };
  while (true) {
    // Find the lowest cell
    size_t min_index = 0;
    for (size_t module = 0; module < PLUTUS_CFG_AFE_TOTAL_CELLS; module++) {
      if (s_plutus.ltc_afe.cell_voltages[module] < s_plutus.ltc_afe.cell_voltages[min_index]) {
        min_index = module;
      }
    }

    // And then balance everything else within +/- of that
    //
    // Our current configuration has { 12, 6, 12, 6 }?
    for (size_t cell_index = 0; cell_index < PLUTUS_CFG_AFE_TOTAL_CELLS; cell_index++) {
      uint16_t min_cell_voltage = s_plutus.ltc_afe.cell_voltages[min_index];
      uint16_t cell_voltage = s_plutus.ltc_afe.cell_voltages[cell_index];

      int16_t cell_difference = cell_voltage - min_cell_voltage;
      if (cell_difference > PLUTUS_MAX_DIFFERENCE_MILLIVOLTS) {
        ltc_afe_toggle_cell_discharge(&s_plutus.ltc_afe, cell_index, true);

        // Ensure we only balance 1 module on each AFE device that we are responsible for
        // The cell_index++ will increment to the next AFE device afterwards
        if (cell_index <= 11) {
          cell_index = 11;
        } else if (cell_index <= 17) {
          cell_index = 17;
        } else if (cell_index <= 29) {
          cell_index = 29;
        } else {
          cell_index = PLUTUS_CFG_AFE_TOTAL_CELLS;
        }
      } else {
        ltc_afe_toggle_cell_discharge(&s_plutus.ltc_afe, cell_index, false);
      }
    }
  }
}
