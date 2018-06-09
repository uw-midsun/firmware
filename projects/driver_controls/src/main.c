
#include <stdint.h>
#include <stdio.h>

#include <stdbool.h>
#include "adc.h"
#include "ads1015.h"
#include "ads1015_def.h"
#include "delay.h"
#include "event_arbiter.h"
#include "event_queue.h"
#include "gpio_it.h"
#include "i2c.h"
#include "input_event.h"
#include "interrupt.h"
#include "log.h"
#include "magnetic_brake_event_generator.h"
#include "soft_timer.h"
#include "status.h"
#include "unity.h"

static MagneticCalibrationData data;

static MagneticBrakeSettings brake_settings = {
  .percentage_threshold = 500,
  .zero_value = 2,
  .hundred_value = 1635,
  .min_allowed_range = 0,
  .max_allowed_range = (1 << 12),
};

static void prv_callback_channel(Ads1015Channel channel, void *context) {
  Ads1015Storage *storage = context;

  ads1015_read_raw(storage, channel, &data.reading);

  MagneticBrakeSettings brake_settings = {
    .percentage_threshold = 500,
    .zero_value = 0,
    .hundred_value = 1635,
    .min_allowed_range = 0,
    .max_allowed_range = (1 << 12),
  };

  int16_t percentage = percentage_converter(&data, &brake_settings);
  int16_t reading = data.reading;

  printf("%d %d\n", data.reading, percentage);

  data.percentage = percentage;
}

int main(void) {
  Ads1015Storage storage;

  gpio_init();
  interrupt_init();
  gpio_it_init();
  soft_timer_init();

  I2CSettings i2c_settings = {
    .speed = I2C_SPEED_FAST,
    .scl = { .port = GPIO_PORT_B, .pin = 8 },
    .sda = { .port = GPIO_PORT_B, .pin = 9 },
  };

  i2c_init(I2C_PORT_2, &i2c_settings);

  GPIOAddress ready_pin = { .port = GPIO_PORT_A, .pin = 10 };

  ads1015_init(&storage, I2C_PORT_1, ADS1015_ADDRESS_GND, &ready_pin);

  magnetic_brake_event_generator_init(&data, &brake_settings, ADS1015_CHANNEL_1);

  ads1015_configure_channel(&storage, ADS1015_CHANNEL_0, true, prv_callback_channel, &storage);

  while (true) {
  }
  return 0;
}
