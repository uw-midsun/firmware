#include <stdio.h>

#include "adc.h"
#include "interrupt.h"
#include "log.h"
#include "gpio.h"

void test_callback(ADCChannel adc_channel, uint16_t reading, void *context) {
  *(uint16_t*)context = reading;
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

  uint16_t adc_readings[16];
  memset(adc_readings, 0, sizeof(uint16_t[16]));

  adc_init(ADC_MODE_SINGLE);

  adc_set_channel(10, 1);
  adc_set_channel(11, 1);
  adc_set_channel(12, 1);

  adc_register_callback(10, test_callback, &adc_readings[10]);
  adc_register_callback(11, test_callback, &adc_readings[11]);
  adc_register_callback(12, test_callback, &adc_readings[12]);

  while (1) {
    adc_read_value(10);

    LOG_DEBUG("{");
    for (int i = 0; i < 16; i++) {
      printf(" %d ", adc_readings[i]);
    }
    printf("}\n");
  }
}
