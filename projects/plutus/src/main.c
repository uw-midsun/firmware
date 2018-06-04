#include <stddef.h>

#include "delay.h"
#include "log.h"
#include "plutus_cfg.h"

#include "plutus_sys.h"

static PlutusSysStorage s_plutus;

int main(void) {
  plutus_sys_init(&s_plutus, PLUTUS_SYS_TYPE_MASTER);

  while (true) {
    uint16_t voltages[PLUTUS_CFG_AFE_TOTAL_CELLS] = { 0 };
    StatusCode status = ltc_afe_read_all_voltage(&s_plutus.ltc_afe, voltages, PLUTUS_CFG_AFE_TOTAL_CELLS);
    if (status != STATUS_CODE_OK) {
      LOG_DEBUG("Invalid status %d\n", status);
    }

    uint16_t aux_voltages[PLUTUS_CFG_AFE_TOTAL_CELLS] = { 0 };
    status = ltc_afe_read_all_aux(&s_plutus.ltc_afe, aux_voltages, PLUTUS_CFG_AFE_TOTAL_CELLS);
    if (status != STATUS_CODE_OK) {
      LOG_DEBUG("Invalid status (aux) %d\n", status);
    }

    for (int i = 0; i < PLUTUS_CFG_AFE_TOTAL_CELLS; i++) {
      LOG_DEBUG("C%d: cell %d.%dmV, aux %d.%dmV\n", i, voltages[i] / 10, voltages[i] % 10,
                aux_voltages[i] / 10, aux_voltages[i] % 10);
    }

    delay_s(1);
  }
}
