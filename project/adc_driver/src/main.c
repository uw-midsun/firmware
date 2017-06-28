#include <stdio.h>

#include "adc.h"
#include "interrupt.h"
#include "log.h"
#include "gpio.h"

void test_callback(ADCChannel adc_channel, uint16_t reading, void *context) {
  uint16_t* adc_reading = (uint16_t*)context;
  *adc_reading = reading;
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

  uint16_t adc_readings[NUM_ADC_CHANNEL];
  memset(adc_readings, 0, sizeof(adc_readings));

  adc_init(ADC_MODE_CONTINUOUS);

  //adc_set_channel(ADC_CHANNEL_10, 1);
  //adc_set_channel(ADC_CHANNEL_11, 1);
  //adc_set_channel(ADC_CHANNEL_12, 1);
  //adc_set_channel(ADC_CHANNEL_13, 1);
  //adc_set_channel(ADC_CHANNEL_TEMP, 1);
  adc_set_channel(ADC_CHANNEL_REF, 1);

  //adc_register_callback(ADC_CHANNEL_10, test_callback, &adc_readings[ADC_CHANNEL_10]);
  //adc_register_callback(ADC_CHANNEL_11, test_callback, &adc_readings[ADC_CHANNEL_11]);
  //adc_register_callback(ADC_CHANNEL_12, test_callback, &adc_readings[ADC_CHANNEL_12]);
  //adc_register_callback(ADC_CHANNEL_13, test_callback, &adc_readings[ADC_CHANNEL_13]);
  //adc_register_callback(ADC_CHANNEL_TEMP, test_callback, &adc_readings[ADC_CHANNEL_TEMP]);
  adc_register_callback(ADC_CHANNEL_REF, test_callback, &adc_readings[ADC_CHANNEL_REF]);

  while (1) {
    LOG_DEBUG("{");
    for (int i = ADC_CHANNEL_0; i < NUM_ADC_CHANNEL; i++) {
      printf(" %d ", adc_readings[i]);
    }
    printf("}\n");
  }
}
