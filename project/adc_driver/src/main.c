#include "adc.h"
#include "gpio.h"

int main() {
  // Set up GPIO pin
  GPIOAddress address = { 0, 0 };
  GPIOSettings settings = { GPIO_DIR_IN, GPIO_STATE_LOW, GPIO_RES_NONE, GPIO_ALTFN_ANALOG };
  
  gpio_init();
  gpio_init_pin(&address, &settings);

  // Initialize adc
  adc_init(ADC_MODE_CONTINUOUS);
  
  if (adc_init_pin(&address, ADC_SAMPLE_RATE_1)) {
    while (1) {
      for (uint32_t i = 0; i < 2000000; i++) {}
      printf("%d mV\n", adc_read(5000));
    }
  }
}
