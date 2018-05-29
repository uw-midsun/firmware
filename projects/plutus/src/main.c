#include <stddef.h>

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

  LtcAfeSettings afe_settings = {
    .mosi = { GPIO_PORT_A, 7 },  //
    .miso = { GPIO_PORT_A, 6 },  //
    .sclk = { GPIO_PORT_A, 5 },  //
    .cs = { GPIO_PORT_A, 4 },    //

    .spi_port = SPI_PORT_1,
    .spi_baudrate = 250000,  //
    .adc_mode = LTC_AFE_ADC_MODE_27KHZ,

    .input_bitset = {
      // PLUTUS_CFG_INPUT_BITSET_FULL,
      PLUTUS_CFG_INPUT_BITSET_SPLIT,
      // PLUTUS_CFG_INPUT_BITSET_FULL,
      // PLUTUS_CFG_INPUT_BITSET_SPLIT,
    }
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
