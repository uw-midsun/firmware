#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "delay.h"
#include "gpio.h"
#include "interrupt.h"
#include "misc.h"
#include "soft_timer.h"

#include "adc.h"
#include "critical_section.h"
#include "log.h"

#include "screen_brightness.h"

static const GPIOAddress gpio_address[] = {
  { GPIO_PORT_B, 2 },
};

static const GPIOAddress adc_address[] = { 
  { GPIO_PORT_A, 0 }, { GPIO_PORT_A, 1 }, { GPIO_PORT_A, 2 },
  { GPIO_PORT_A, 3 }, { GPIO_PORT_A, 4 }, { GPIO_PORT_A, 5 },
  { GPIO_PORT_A, 6 }, { GPIO_PORT_A, 7 }, { GPIO_PORT_B, 0 },
  { GPIO_PORT_A, 1 }, { GPIO_PORT_B, 0 }, { GPIO_PORT_B, 1 },
  { GPIO_PORT_C, 2 }, { GPIO_PORT_C, 3 }, { GPIO_PORT_C, 4 },
  { GPIO_PORT_C, 5 } 
};

static const GPIOSettings gpio_settings = {
    GPIO_DIR_OUT,
    GPIO_STATE_LOW,
    GPIO_ALTFN_NONE, 
    GPIO_RES_NONE        
};

static const GPIOSettings adc_settings = {
    GPIO_DIR_IN,     
    GPIO_STATE_LOW,
    GPIO_RES_NONE,   
    GPIO_ALTFN_ANALOG
  };

void test_callback(ADCChannel adc_channel, void *context) {
  uint16_t *adc_reading = (uint16_t *)context;
  adc_read_converted(adc_channel, adc_reading);
}

void read_brightness(void) {
  //Init the gpio's to be used 
  gpio_init_pin(&gpio_address[0], &gpio_settings);
  gpio_init_pin(&adc_address[0], &adc_settings);

  adc_init(ADC_MODE_CONTINUOUS);

  ADCChannel adc_channel;
  adc_get_channel(adc_address[0], &adc_channel);
  adc_set_channel(adc_channel, true);

  uint16_t reading;
  adc_register_callback(adc_channel, test_callback, &reading);
  
  for (;;) { //infinite for loop 
    adc_read_raw(adc_channel, &reading); //adc_register_callback(adc_channel, test_callback, &reading)
    printf("adc reading: %d",reading);
    printf("\n");
    printf("brightness reading: %d", map_brightness(reading));
    printf("\n");
    delay_ms(100);
  }
}

uint16_t map_brightness(uint16_t adc_reading){
  return ((adc_reading*100)*100)/MAX_BRIGHTNESS;
}


