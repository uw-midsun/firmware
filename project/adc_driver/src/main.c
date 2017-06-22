#include "adc.h"
#include "interrupt.h"
#include "log.h"
#include "stm32f0xx.h"
#include "gpio.h"

#include <stdio.h>

// Broken channels: 2, 3, 6, 7, 8, 9, 

void test_callback(ADCChannel adc_channel, uint16_t reading, void *context) {
  printf("Callback called by channel %d with reading %d0\n", adc_channel, reading);
}

int main() {

  GPIOAddress address[] = {
    { 0, 0 }, { 0, 1 }, { 0, 2 }, { 0, 3 },
    { 0, 4 }, { 0, 5 }, { 0, 6 }, { 0, 7 },
    { 1, 0 }, { 1, 1 }, { 2, 0 }, { 2, 1 },
    { 2, 2 }, { 2, 3 }, { 2, 4 }, { 2, 5 }
  };

  GPIOSettings settings = { GPIO_DIR_IN, GPIO_STATE_LOW, GPIO_RES_NONE, GPIO_ALTFN_ANALOG };

  gpio_init();
  interrupt_init();

  for (uint8_t i = 0; i < 16; i++) {
    gpio_init_pin(&address[i], &settings);
  }
    
  adc_init(ADC_MODE_SINGLE);
  
  adc_set_channel(12, ENABLE);
  adc_set_channel(13, ENABLE);
  adc_set_channel(14, ENABLE);
  adc_set_channel(15, ENABLE); 
  
  /*
  for (int i = 12; i < 16; i++) {
    adc_register_callback(i, test_callback, 0);
  }
  */
  
  while (1) {
    printf("Channel #%d = %d | ", 12, adc_read_value(12));   
    printf("Channel #%d = %d | ", 13, adc_read_value(13));
    printf("Channel #%d = %d | ", 14, adc_read_value(14));
    printf("Channel #%d = %d\n", 15, adc_read_value(15));
  }
}
