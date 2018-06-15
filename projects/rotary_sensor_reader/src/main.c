#include <stdbool.h>
#include <stdio.h>

#include "adc.h"
#include "delay.h"
#include "gpio.h"
#include "interrupt.h"
#include "log.h"
#include "soft_timer.h"

void calibrate_boundary(uint16_t *boundary_reading, ADCChannel channel,
                        uint32_t calibration_time_limit) {
  delay_s(calibration_time_limit);
  adc_read_raw(channel, boundary_reading);
}

int main(void) {
  gpio_init();
  interrupt_init();
  soft_timer_init();

  uint16_t reading = 0;
  uint16_t max_bound = 0;
  uint16_t min_bound = 0;
  uint16_t mid_bound = 0;
  uint32_t calibration_time_limit = 6;
  ADCChannel conversion_channel;

  const GPIOAddress conversion_address = {
    .port = GPIO_PORT_A,
    .pin = 0,
  };

  GPIOSettings analog_settings = {
    .direction = GPIO_DIR_IN,           //
    .state = GPIO_STATE_LOW,            //
    .resistor = GPIO_RES_NONE,          //
    .alt_function = GPIO_ALTFN_ANALOG,  //
  };

  gpio_init_pin(&conversion_address, &analog_settings);

  adc_init(ADC_MODE_CONTINUOUS);
  adc_get_channel(conversion_address, &conversion_channel);
  adc_set_channel(conversion_channel, true);

  printf("Please rotate the wheel in the counterclockwise direction as far as possible \n");
  calibrate_boundary(&min_bound, conversion_channel, calibration_time_limit);
  LOG_DEBUG("read min bound: %d \n", min_bound);

  printf("Please rotate the wheel in the clockwise direction as far as possible \n");
  calibrate_boundary(&max_bound, conversion_channel, calibration_time_limit);
  LOG_DEBUG("read max bound: %d \n", max_bound);

  mid_bound = (max_bound - min_bound) / 2;
  LOG_DEBUG("read max bound: %d \n", mid_bound);

  delay_s(calibration_time_limit);

  printf("Conversion Channel:");
  LOG_DEBUG(" %u \n", conversion_channel);

  while (true) {
    adc_read_raw(conversion_channel, &reading);
    LOG_DEBUG(" %d \n", max_bound / reading);
  }
}
