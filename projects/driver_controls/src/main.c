#include <stdint.h>
#include <stdio.h>

#include "adc.h"
#include "ads1015.h"
#include "ads1015_def.h"
#include "analog_io.h"
#include "delay.h"
#include "digital_io.h"
#include "event_arbiter.h"
#include "gpio_it.h"
#include "i2c.h"
#include "input_event.h"
#include "interrupt.h"
#include "log.h"
#include "unity.h"


static Ads1015Storage s_storage;

static bool s_callback_called[NUM_ADS1015_CHANNELS];

static void prv_callback_channel(Ads1015Channel channel, void *context) {
  bool *s_callback_called = context;
  *s_callback_called = true;
}

static bool prv_channel_reading_void(int16_t reading) {
  return (reading < (ADS1015_CURRENT_FSR / 2)) && (reading >= 0);
}

int main() {

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
 
 ads1015_init(&s_storage, I2C_PORT_2, ADS1015_ADDRESS_VDD, &ready_pin);
 
 ads1015_configure_channel(&s_storage, ADS1015_CHANNEL_0, true, prv_callback_channel,
                            &s_callback_called[ADS1015_CHANNEL_0]);

      int16_t reading = 0;

  ads1015_read_raw(&s_storage, ADS1015_CHANNEL_0, &reading);
}
