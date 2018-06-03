#include <stddef.h>

#include "delay.h"
#include "gpio.h"
#include "interrupt.h"
#include "log.h"
#include "ltc_afe.h"
#include "plutus_cfg.h"
#include "soft_timer.h"
#include "spi.h"
#include "sequenced_relay.h"

static LtcAfeStorage s_afe;
static SequencedRelayStorage s_relays;

int main(void) {
  gpio_init();
  interrupt_init();
  soft_timer_init();

  const SequencedRelaySettings relay_settings = {
    .can_message = SYSTEM_CAN_MESSAGE_BATTERY_RELAY_MAIN,
    .left_relay = PLUTUS_CFG_RELAY_PWR,
    .right_relay = PLUTUS_CFG_RELAY_GND,
    .delay_ms = PLUTUS_CFG_RELAY_DELAY_MS,
  };
  sequenced_relay_init(&s_relays, &relay_settings);

  const LtcAfeSettings afe_settings = {
    .mosi = PLUTUS_CFG_AFE_SPI_MOSI,
    .miso = PLUTUS_CFG_AFE_SPI_MISO,
    .sclk = PLUTUS_CFG_AFE_SPI_SCLK,
    .cs = PLUTUS_CFG_AFE_SPI_CS,

    .spi_port = PLUTUS_CFG_AFE_SPI_PORT,
    .spi_baudrate = PLUTUS_CFG_AFE_SPI_BAUDRATE,
    .adc_mode = PLUTUS_CFG_AFE_MODE,

    .cell_bitset = PLUTUS_CFG_CELL_BITSET_ARR,
    .aux_bitset = PLUTUS_CFG_AUX_BITSET_ARR,
  };

  ltc_afe_init(&s_afe, &afe_settings);

  while (true) {
    uint16_t voltages[PLUTUS_CFG_AFE_TOTAL_CELLS] = { 0 };
    StatusCode status = ltc_afe_read_all_voltage(&s_afe, voltages, PLUTUS_CFG_AFE_TOTAL_CELLS);
    if (status != STATUS_CODE_OK) {
      LOG_DEBUG("Invalid status %d\n", status);
    }

    uint16_t aux_voltages[PLUTUS_CFG_AFE_TOTAL_CELLS] = { 0 };
    status = ltc_afe_read_all_aux(&s_afe, aux_voltages, PLUTUS_CFG_AFE_TOTAL_CELLS);
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
