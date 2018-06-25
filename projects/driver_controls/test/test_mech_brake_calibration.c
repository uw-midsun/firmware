
// Note : Although this is structured as a test, it is the actual calibration sequence for the
// mechanical brake. It is meant to be used only once to retreive the values when the brake is
// pressed and unpressed.

#include "ads1015.h"
#include "dc_calib.h"
#include "dc_cfg.h"
#include "delay.h"
#include "event_arbiter.h"
#include "gpio.h"
#include "gpio_it.h"
#include "i2c.h"
#include "input_event.h"
#include "interrupt.h"
#include "log.h"
#include "mech_brake.h"
#include "mech_brake_calibration.h"
#include "soft_timer.h"
#include "test_helpers.h"
#include "unity.h"

static Ads1015Storage s_ads1015_storage;
static MechBrakeStorage s_mech_brake_storage;
static DcCalibBlob s_calib_blob;
static MechBrakeCalibrationStorage s_calibration_storage;

void setup_test(void) {
  gpio_init();
  interrupt_init();
  gpio_it_init();
  soft_timer_init();

  I2CSettings i2c_settings = {
    .speed = I2C_SPEED_FAST,
    .scl = DC_CFG_I2C_BUS_SCL,
    .sda = DC_CFG_I2C_BUS_SDA,
  };

  i2c_init(DC_CFG_I2C_BUS_PORT, &i2c_settings);

  GPIOAddress ready_pin = DC_CFG_PEDAL_ADC_RDY_PIN;

  const MechBrakeSettings calib_settings = {
    .ads1015 = &s_ads1015_storage,
    .channel = ADS1015_CHANNEL_2,
  };

  event_queue_init();
  ads1015_init(&s_ads1015_storage, DC_CFG_I2C_BUS_PORT, DC_CFG_PEDAL_ADC_ADDR, &ready_pin);

  TEST_ASSERT_OK(calib_init(&s_calib_blob, sizeof(s_calib_blob)));
  mech_brake_calibration_init(&s_calibration_storage, &calib_settings);
}

void teardown_test(void) {}

void test_mech_brake_calibration_run(void) {
  LOG_DEBUG("Please ensure the brake is not being pressed.\n");
  delay_s(7);
  LOG_DEBUG("Beginning sampling\n");
  mech_brake_sample(&s_calibration_storage, MECH_BRAKE_CALIBRATION_POINT_UNPRESSED);
  LOG_DEBUG("Completed sampling\n");
  LOG_DEBUG("Please press and hold the brake\n");
  delay_s(7);
  LOG_DEBUG("Beginning sampling\n");
  mech_brake_sample(&s_calibration_storage, MECH_BRAKE_CALIBRATION_POINT_PRESSED);
  LOG_DEBUG("Completed sampling\n");

  mech_brake_get_calib_data(&s_calibration_storage, &s_calib_blob.mech_brake_calib);

  LOG_DEBUG("%d %d\n", s_calib_blob.mech_brake_calib.zero_value,
            s_calib_blob.mech_brake_calib.hundred_value);
}

void test_mech_brake_calibration_verify(void) {
  const MechBrakeSettings calib_settings = {
    .ads1015 = &s_ads1015_storage,
    .channel = ADS1015_CHANNEL_2,
  };

  TEST_ASSERT_OK(
      mech_brake_init(&s_mech_brake_storage, &calib_settings, &s_calib_blob.mech_brake_calib));
  Event e = { 0 };
  while (true) {
    if (status_ok(event_process(&e))) {
      if (e.id == INPUT_EVENT_MECHANICAL_BRAKE_PRESSED) {
        LOG_DEBUG("Pressed: %d\n", e.data);
      } else if (e.id == INPUT_EVENT_MECHANICAL_BRAKE_RELEASED) {
        LOG_DEBUG("Released: %d\n", e.data);
      }
    }
  }
}
