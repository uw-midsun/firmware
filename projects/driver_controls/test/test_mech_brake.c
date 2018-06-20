
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

static MechBrakeCalibrationData calib_data = {
  .zero_value = 513,
  .hundred_value = 624,
};

void setup_test(void) {
  Ads1015Storage storage;
  gpio_init();
  interrupt_init();
  gpio_it_init();
  soft_timer_init();

  MechBrakeSettings brake_settings = {
    .percentage_threshold = 500,
    .min_allowed_range = 0,
    .max_allowed_range = (1 << 12),
    .channel = ADS1015_CHANNEL_2,
    .ads1015 = &storage,
  };

  I2CSettings i2c_settings = {
    .speed = I2C_SPEED_FAST,
    .scl = { .port = GPIO_PORT_B, .pin = 8 },
    .sda = { .port = GPIO_PORT_B, .pin = 9 },
  };

  i2c_init(I2C_PORT_2, &i2c_settings);

  GPIOAddress ready_pin = {.port = GPIO_PORT_A, .pin = 10 };

  ads1015_init(&storage, I2C_PORT_2, ADS1015_ADDRESS_GND, &ready_pin);

  mech_brake_init(&mech_brake_storage, &brake_settings, &calib_data);
}

void teardown_test(void) {}

void test_mech_brake_loop(void) {
  while (true) {
  }
}
