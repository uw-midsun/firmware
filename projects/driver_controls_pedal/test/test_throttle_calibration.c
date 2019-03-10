#include "ads1015.h"
#include "calib.h"
#include "crc32.h"
#include "delay.h"
#include "event_queue.h"
#include "flash.h"
#include "gpio.h"
#include "gpio_it.h"
#include "i2c.h"
#include "interrupt.h"
#include "log.h"
#include "pc_calib.h"
#include "soft_timer.h"
#include "throttle.h"
#include "throttle_calibration.h"
#include "unity.h"

static Ads1015Storage s_ads1015_storage;
static ThrottleStorage s_throttle_storage;
static ThrottleCalibrationStorage s_calibration_storage;
static DcCalibBlob s_calib_blob;

void setup_test(void) {
  gpio_init();
  interrupt_init();
  gpio_it_init();
  soft_timer_init();
  crc32_init();
  flash_init();

  I2CSettings i2c_settings = {
    .speed = I2C_SPEED_FAST,                   //
    .sda = { .port = GPIO_PORT_B, .pin = 9 },  //
    .scl = { .port = GPIO_PORT_B, .pin = 8 },  //
  };
  i2c_init(I2C_PORT_1, &i2c_settings);
  GpioAddress ready_pin = {
    .port = GPIO_PORT_A,  //
    .pin = 10,            //
  };
  event_queue_init();
  ads1015_init(&s_ads1015_storage, I2C_PORT_1, ADS1015_ADDRESS_GND, &ready_pin);

  ThrottleCalibrationSettings calib_settings = {
    .ads1015 = &s_ads1015_storage,
    .adc_channel = { ADS1015_CHANNEL_0, ADS1015_CHANNEL_1 },
    .zone_percentage = { 40, 10, 50 },
    .sync_safety_factor = 2,
    .bounds_tolerance_percentage = 1,
  };
  throttle_calibration_init(&s_calibration_storage, &calib_settings);

  calib_init(&s_calib_blob, sizeof(s_calib_blob), true);
}

void teardown_test(void) {}

void test_throttle_calibration_run(void) {
  LOG_DEBUG("Please move to full brake.\n");
  delay_s(5);
  LOG_DEBUG("Beginning sampling\n");
  throttle_calibration_sample(&s_calibration_storage, THROTTLE_CALIBRATION_POINT_FULL_BRAKE);
  LOG_DEBUG("Completed sampling\n");
  LOG_DEBUG("Please move to full accel.\n");
  delay_s(5);
  LOG_DEBUG("Beginning sampling\n");
  throttle_calibration_sample(&s_calibration_storage, THROTTLE_CALIBRATION_POINT_FULL_ACCEL);
  LOG_DEBUG("Completed sampling\n");

  DcCalibBlob *dc_calib_blob = calib_blob();
  throttle_calibration_result(&s_calibration_storage, &dc_calib_blob->throttle_calib);

  LOG_DEBUG("Stored throttle calib data\n");
  calib_commit();

  throttle_init(&s_throttle_storage, &dc_calib_blob->throttle_calib, &s_ads1015_storage);

  while (true) {
    ThrottlePosition position = { .zone = NUM_THROTTLE_ZONES };
    throttle_get_position(&s_throttle_storage, &position);
    const char *zone[] = { "Brake", "Coast", "Accel", "Invalid" };
    printf("%s: %d/%d (%d)\n", zone[position.zone], position.numerator, THROTTLE_DENOMINATOR,
           position.numerator * 100 / THROTTLE_DENOMINATOR);
  }
}
