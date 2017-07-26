#include <stddef.h>

#include "gpio.h"
#include "ltc_afe.h"
#include "log.h"
#include "spi.h"
#include "plutus_config.h"

int main(void) {
  gpio_init();
  interrupt_init();
  soft_timer_init();

  LTCAFESettings afe_settings = {
    .mosi = { GPIO_PORT_A, 7 },
    .miso = { GPIO_PORT_A, 6 },
    .sclk = { GPIO_PORT_A, 5 },
    .cs = { GPIO_PORT_A, 4 },

    .spi_port = 0, // to make the build pass on x86,
    .spi_baudrate = 250000,
    .adc_mode = LTC_AFE_ADC_MODE_27KHZ
  };

  ltc_afe_init(&afe_settings);

  while (true) {
    uint16_t voltage_data[12 * LTC_AFE_DEVICES_IN_CHAIN] = { 0 };
    size_t voltage_len = 12 * LTC_AFE_DEVICES_IN_CHAIN;
    StatusCode status = ltc_afe_read_all_voltage(&afe_settings, voltage_data, voltage_len);
    if (status != STATUS_CODE_OK) {
      LOG_DEBUG("Invalid status\n");
    }
  }
}
