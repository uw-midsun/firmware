#include <stddef.h>

#include "delay.h"
#include "gpio.h"
#include "interrupt.h"
#include "log.h"
#include "ltc2484.h"
#include "ltc_afe.h"
#include "plutus_config.h"
#include "soft_timer.h"
#include "spi.h"

int main(void) {
  gpio_init();
  interrupt_init();
  soft_timer_init();

  /* LtcAfeSettings afe_settings = { */
  /* .mosi = { GPIO_PORT_A, 7 },  // */
  /* .miso = { GPIO_PORT_A, 6 },  // */
  /* .sclk = { GPIO_PORT_A, 5 },  // */
  /* .cs = { GPIO_PORT_A, 4 },    // */

  /* .spi_port = 0,           // to make build pass on x86 */
  /* .spi_baudrate = 250000,  // */
  /* .adc_mode = LTC_AFE_ADC_MODE_27KHZ, */
  /* }; */

  /* ltc_afe_init(&afe_settings); */

  /* while (true) { */
  /* uint16_t voltages[LTC6804_CELLS_PER_DEVICE * PLUTUS_AFE_DEVICES_IN_CHAIN] = { 0 }; */
  /* size_t len = SIZEOF_ARRAY(voltages); */
  /* StatusCode status = ltc_afe_read_all_voltage(&afe_settings, voltages, len); */
  /* if (status != STATUS_CODE_OK) { */
  /* LOG_DEBUG("Invalid status\n"); */
  /* } */
  /* } */

  Ltc2484Settings adc_settings = { .mosi = { GPIO_PORT_B, 15 },
                                   .miso = { GPIO_PORT_B, 14 },
                                   .sclk = { GPIO_PORT_B, 13 },
                                   .cs = { GPIO_PORT_B, 12 },

                                   .spi_port = 1,           // to make build pass on x86
                                   .spi_baudrate = 250000,  //

                                   .filter_mode = LTC_2484_FILTER_50HZ_60HZ };
  ltc2484_init(&adc_settings);

  while (true) {
    int32_t value = 0;
    ltc2484_read(&adc_settings, &value);
  }
}
