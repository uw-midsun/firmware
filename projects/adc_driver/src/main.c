#include <stdio.h>
#include <string.h>

#include "adc.h"
#include "critical_section.h"
#include "gpio.h"
#include "interrupt.h"
#include "log.h"

void test_callback(ADCChannel adc_channel, void *context) {
  uint16_t *adc_reading = (uint16_t *)context;
  adc_read_converted(adc_channel, adc_reading);
}

int main() {
  GpioAddress address[] = { { GPIO_PORT_A, 0 }, { GPIO_PORT_A, 1 }, { GPIO_PORT_A, 2 },
                            { GPIO_PORT_A, 3 }, { GPIO_PORT_A, 4 }, { GPIO_PORT_A, 5 },
                            { GPIO_PORT_A, 6 }, { GPIO_PORT_A, 7 }, { GPIO_PORT_B, 0 },
                            { GPIO_PORT_A, 1 }, { GPIO_PORT_B, 0 }, { GPIO_PORT_B, 1 },
                            { GPIO_PORT_C, 2 }, { GPIO_PORT_C, 3 }, { GPIO_PORT_C, 4 },
                            { GPIO_PORT_C, 5 } };

  GpioSettings settings = {
    GPIO_DIR_IN,        //
    GPIO_STATE_LOW,     //
    GPIO_RES_NONE,      //
    GPIO_ALTFN_ANALOG,  //
  };

  gpio_init();
  interrupt_init();

  for (uint8_t i = 0; i < 16; i++) {
    gpio_init_pin(&address[i], &settings);
  }

  uint16_t adc_readings[NUM_ADC_CHANNELS];
  memset(adc_readings, 0, sizeof(adc_readings));

  adc_init(ADC_MODE_SINGLE);

  for (ADCChannel i = ADC_CHANNEL_10; i < ADC_CHANNEL_14; i++) {
    adc_set_channel(i, true);
    adc_register_callback(i, test_callback, &adc_readings[i]);
  }

  adc_set_channel(ADC_CHANNEL_TEMP, true);
  adc_set_channel(ADC_CHANNEL_BAT, true);

  adc_register_callback(ADC_CHANNEL_TEMP, test_callback, &adc_readings[ADC_CHANNEL_TEMP]);
  adc_register_callback(ADC_CHANNEL_REF, test_callback, &adc_readings[ADC_CHANNEL_REF]);
  adc_register_callback(ADC_CHANNEL_BAT, test_callback, &adc_readings[ADC_CHANNEL_BAT]);

  uint16_t reading;

  for (;;) {
    adc_read_raw(ADC_CHANNEL_10, &reading);
    LOG_DEBUG("{");
    for (int i = ADC_CHANNEL_0; i < ADC_CHANNEL_TEMP; i++) {
      printf(" %d ", adc_readings[i]);
    }
    for (int i = ADC_CHANNEL_TEMP; i < NUM_ADC_CHANNELS; i++) {
      printf(" %d ", adc_readings[i]);
    }
    printf("}\n");
  }
}
