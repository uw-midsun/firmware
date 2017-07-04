#include <stdio.h>

#include "adc.h"
#include "interrupt.h"
#include "log.h"
#include "critical_section.h"
#include "gpio.h"

void test_callback(ADCChannel adc_channel, void *context) {
  uint16_t* adc_reading = (uint16_t*)context;
  adc_read_converted(adc_channel, adc_reading);
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

  adc_init(ADC_MODE_SINGLE);

  for (ADCChannel i = ADC_CHANNEL_10; i < ADC_CHANNEL_14; i++) {
    adc_set_channel(i, 1);
    adc_register_callback(i, test_callback, &adc_readings[i]);
  }

  adc_set_channel(ADC_CHANNEL_TEMP, 1);
  adc_set_channel(ADC_CHANNEL_BAT, 1);

  adc_register_callback(ADC_CHANNEL_TEMP, test_callback, &adc_readings[ADC_CHANNEL_TEMP]);
  adc_register_callback(ADC_CHANNEL_REF, test_callback, &adc_readings[ADC_CHANNEL_REF]);
  adc_register_callback(ADC_CHANNEL_BAT, test_callback, &adc_readings[ADC_CHANNEL_BAT]);

  uint16_t reading;

  while (1) {
    adc_read_raw(ADC_CHANNEL_10, &reading);
    LOG_DEBUG("{");
    for (int i = ADC_CHANNEL_0; i < ADC_CHANNEL_TEMP; i++) {
      printf(" %d ", adc_readings[i]);
    }
    for (int i = ADC_CHANNEL_TEMP; i < NUM_ADC_CHANNEL; i++) {
      adc_readings[i];
      printf(" %d ", adc_readings[i]);
    }
    printf("}\n");
  }
}
