#include "gpio.h"
#include "ltc_afe.h"
#include "log.h"
#include "spi.h"

int main(void) {
  gpio_init();
  interrupt_init();
  soft_timer_init();

  LtcAfeSettings afe_settings = {
    .mosi = { GPIO_PORT_B, 15 },
    .miso = { GPIO_PORT_B, 14 },
    .sclk = { GPIO_PORT_B, 13 },
    .cs = { GPIO_PORT_C, 0 },

    .spi_port = 1,
    .adc_mode = LTC_AFE_ADC_MODE_27KHZ,
    .devices_in_chain = 1
  };

  printf("Setting up AFE\n");
  LtcAfe_init(&afe_settings);

  printf("Done setting up AFE\n");

  while (true) {
    StatusCode status = LtcAfe_read_config(&afe_settings);
    if (status != STATUS_CODE_OK) {
      printf("Invalid status\n");
    }
  }
}
