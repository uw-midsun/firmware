#include <stddef.h>

#include "gpio.h"
#include "log.h"
#include "ltc_afe.h"
#include "plutus_config.h"
#include "spi.h"

int main(void) {
  gpio_init();
  interrupt_init();
  soft_timer_init();

  LtcAfeSettings afe_settings = {
    .mosi = { GPIO_PORT_A, 7 },  //
    .miso = { GPIO_PORT_A, 6 },  //
    .sclk = { GPIO_PORT_A, 5 },  //
    .cs = { GPIO_PORT_A, 4 },    //

    .spi_port = 0,           //
    .spi_baudrate = 250000,  //
    .adc_mode = LTC_AFE_ADC_MODE_27KHZ,
  };

  ltc_afe_init(&afe_settings);

  while (true) {
    uint16_t voltages[LTC6804_CELLS_PER_DEVICE * PLUTUS_AFE_DEVICES_IN_CHAIN] = { 0 };
    size_t len = SIZEOF_ARRAY(voltages);
    StatusCode status = ltc_afe_read_all_voltage(&afe_settings, voltages, len);
    if (status != STATUS_CODE_OK) {
      LOG_DEBUG("Invalid status\n");
    }
  }
}
