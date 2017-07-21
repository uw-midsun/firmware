#include <stdio.h>
#include <stdint.h>

#include "adc.h"
#include "gpio.h"

#include "digital_io.h"
#include "analog_io.h"

int main() {
  // Initialize the various driver control devices
  gpio_init();
  interrupt_init();
  gpio_it_init();

  adc_init(ADC_MODE_CONTINUOUS);

  digital_io_init();
  analog_io_init();

  event_queue_init();
}
