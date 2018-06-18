
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
#include "mech_brake.h"
#include "soft_timer.h"
#include "status.h"
#include "unity.h"

static MechBrakeStorage mech_brake_storage;

static MechBrakeSettings brake_settings = {
  .percentage_threshold = 500,
  .min_allowed_range = 0,
  .max_allowed_range = (1 << 12),
  .channel = ADS1015_CHANNEL_2,
};

static MechBrakeCalibrationData calib_data = {
  .zero_value = 513,
  .hundred_value = 624,
};

static void prv_callback_channel(Ads1015Channel channel, void *context) {
  MechBrakeStorage *storage = context;

  ads1015_read_raw(storage->settings.ads1015, channel, &mech_brake_storage.reading);

  int16_t percentage = percentage_converter(&mech_brake_storage);
  int16_t reading = mech_brake_storage.reading;

  printf("%d %d\n", mech_brake_storage.reading, percentage);

  mech_brake_storage.percentage = percentage;
}

int main(void) {

  gpio_init();
  interrupt_init();
  gpio_it_init();
  soft_timer_init();

  I2CSettings i2c_settings = {
    .speed = I2C_SPEED_FAST,
    .scl = { .port = GPIO_PORT_B, .pin = 8 },
    .sda = { .port = GPIO_PORT_B, .pin = 9 },
  };

  i2c_init(I2C_PORT_1, &i2c_settings);

  GPIOAddress ready_pin = { .port = GPIO_PORT_A, .pin = 10 };

  mech_brake_init(&mech_brake_storage, &brake_settings, &calib_data);

  ads1015_init(mech_brake_storage.settings.ads1015, I2C_PORT_1, ADS1015_ADDRESS_GND, &ready_pin);

  ads1015_configure_channel(mech_brake_storage.settings.ads1015, mech_brake_storage.settings.channel, true, prv_callback_channel, &mech_brake_storage);

  while (true) {
  }

  return 0;
}
