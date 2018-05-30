#include <stddef.h>

#include "delay.h"
#include "gpio.h"
#include "interrupt.h"
#include "log.h"
#include "ltc_afe.h"
#include "plutus_cfg.h"
#include "soft_timer.h"
#include "spi.h"

static LtcAfeStorage s_afe;

int main(void) {
  gpio_init();
  interrupt_init();
  soft_timer_init();

  const LtcAfeSettings afe_settings = {
    .mosi = PLUTUS_CFG_SPI_MOSI,  //
    .miso = PLUTUS_CFG_SPI_MISO,  //
    .sclk = PLUTUS_CFG_SPI_SCLK,  //
    .cs = PLUTUS_CFG_SPI_CS,      //

    .spi_port = PLUTUS_CFG_SPI_PORT,          //
    .spi_baudrate = PLUTUS_CFG_SPI_BAUDRATE,  //
    .adc_mode = PLUTUS_CFG_AFE_MODE,

    .input_bitset = PLUTUS_CFG_INPUT_BITSET_ARR,
  };

  ltc_afe_init(&s_afe, &afe_settings);

  while (true) {
    uint16_t voltages[PLUTUS_CFG_TOTAL_CELLS] = { 0 };
    size_t len = SIZEOF_ARRAY(voltages);
    StatusCode status = ltc_afe_read_all_voltage(&s_afe, voltages, len);
    if (status != STATUS_CODE_OK) {
      LOG_DEBUG("Invalid status\n");
    }

    for (size_t i = 0; i < PLUTUS_CFG_TOTAL_CELLS; i++) {
      LOG_DEBUG("C%d: %d\n", i, voltages[i]);
    }
  }
}
