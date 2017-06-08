#include "adc.h"
#include "gpio.h"

int main() {
  GPIOAddress address[] = { 
	{ 0, 0 }, { 0, 1 }, { 0, 2 }, { 0, 3 },
	{ 0, 4 }, { 0, 5 }, { 0, 6 }, { 0, 7 },
	{ 1, 0 }, { 1, 1 }, { 2, 0 }, { 2, 1 },
	{ 2, 2 }, { 2, 3 }, { 2, 4 }, { 2, 5 }
  };
  
  GPIOSettings settings = { GPIO_DIR_IN, GPIO_STATE_LOW, GPIO_RES_NONE, GPIO_ALTFN_ANALOG };
  ADCSettings adc_settings = { ADC_MODE_CONTINUOUS, ADC_RESOLUTION_12 };
  gpio_init();
  
  for (uint8_t i = 0; i < 16; i++) {
  	gpio_init_pin(&address[i], &settings);
  }

  adc_init(&adc_settings);
	
  for (uint8_t i = 0; i < 16; i++) {
    adc_init_pin(&address[i], ADC_SAMPLE_RATE_1);
  }
  
  while (1) {
    for (uint32_t i = 0; i < 2000000; i++) {}
    printf("%d mv\n", adc_read(&address[0], 200));
  }
}
