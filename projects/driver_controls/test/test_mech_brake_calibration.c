
// Note : Although this is structured as a test, it is the actual calibration sequence for the
// mechanical brake. It is meant to be used only once to retreive the values when the brake is
// pressed and unpressed.

#include "ads1015.h"
#include "delay.h"
#include "event_arbiter.h"
#include "gpio.h"
#include "gpio_it.h"
#include "i2c.h"
#include "interrupt.h"
#include "log.h"
#include "mech_brake.h"
#include "mech_brake_calibration.h"
#include "soft_timer.h"
#include "unity.h"

static Ads1015Storage s_ads1015_storage;
static MechBrakeStorage mech_brake_storage;
static MechBrakeCalibrationStorage s_calibration_storage;
Ads1015Storage storage;

void setup_test(void) {
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

  GPIOAddress ready_pin = {
    .port = GPIO_PORT_A,
    .pin = 10,
  };

   const MechBrakeSettings calib_settings = {
    .ads1015 = &storage,
    .channel = ADS1015_CHANNEL_2,
    .min_allowed_range = 0,
    .max_allowed_range = (1 << 12),
  };

  event_queue_init();
  ads1015_init(&storage, I2C_PORT_1, ADS1015_ADDRESS_GND, &ready_pin);

  mech_brake_calibration_init(&s_calibration_storage, &calib_settings);
}

void teardown_test(void) {}

void test_mech_brake_calibration_run(void) {
  LOG_DEBUG("Please ensure the brake is not being pressed.\n");
  delay_s(5);
  LOG_DEBUG("Beginning sampling\n");
  mech_brake_sample(&s_calibration_storage, MECH_BRAKE_CALIBRATION_POINT_UNPRESSED);
  LOG_DEBUG("Completed sampling\n");
  LOG_DEBUG("Please press and hold the brake\n");
  delay_s(5);
  LOG_DEBUG("Beginning sampling\n");
  mech_brake_sample(&s_calibration_storage, MECH_BRAKE_CALIBRATION_POINT_PRESSED);
  LOG_DEBUG("Completed sampling\n");
  
  MechBrakeCalibrationData calib_data;
  mech_brake_get_calib_data(&s_calibration_storage, &calib_data);

  LOG_DEBUG("%d %d\n", calib_data.zero_value, calib_data.hundred_value);

  while (true) {
  }
}
