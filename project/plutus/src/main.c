#include "gpio.h"
#include "ltc_afe.h"
#include "log.h"
#include "spi.h"

int main(void) {
  gpio_init();
  interrupt_init();
  soft_timer_init();

  LtcAfeSettings afe_settings = {
    .mosi = { GPIO_PORT_A, 7 },
    .miso = { GPIO_PORT_A, 6 },
    .sclk = { GPIO_PORT_A, 5 },
    .cs = { GPIO_PORT_A, 4 },

    .spi_port = SPI_PORT_1,
    .adc_mode = LTC_AFE_ADC_MODE_27KHZ,
    .devices_in_chain = 1
  };

  LtcAfe_init(&afe_settings);

  while (true) {
    StatusCode status = LtcAfe_read_config(&afe_settings);
    if (status != STATUS_CODE_OK) {
      LOG_DEBUG("Invalid status\n");
    }
  }
}
