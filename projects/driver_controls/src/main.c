#include <stdint.h>
#include <stdio.h>

#include "adc.h"
#include "ads1015.h"
#include "ads1015_def.h"
#include "delay.h"
#include "event_arbiter.h"
#include "gpio_it.h"
#include "i2c.h"
#include "input_event.h"
#include "interrupt.h"
#include "log.h"
#include "soft_timer.h"
#include "unity.h"
#include <stdbool.h>



static void prv_callback_channel(Ads1015Channel channel, void *context) {
  Ads1015Storage *storage = context;
  int16_t reading = 0;
  ads1015_read_raw(storage, channel, &reading);
  printf("%d\n", reading);
}

int main(void) {
  Ads1015Storage storage;

  gpio_init();
  interrupt_init();
  gpio_it_init();
  soft_timer_init();
  I2CSettings i2c_settings = {
    .speed = I2C_SPEED_FAST,
    .scl = { .port = GPIO_PORT_B, .pin = 10 },
    .sda = { .port = GPIO_PORT_B, .pin = 11 },
  };
  i2c_init(I2C_PORT_2, &i2c_settings);
  GPIOAddress ready_pin = { .port = GPIO_PORT_A, .pin = 9 };
 
  ads1015_init(&storage, I2C_PORT_2, ADS1015_ADDRESS_GND, &ready_pin);
 
  ads1015_configure_channel(&storage, ADS1015_CHANNEL_0, true, prv_callback_channel,
                            &storage);

  
  while (true) {
  }

  return 0;
}
