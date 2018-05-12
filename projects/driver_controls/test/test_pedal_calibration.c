// This is technically not a test but the actual calibration environment!
// There are 4 stages of calibration: start, full brake, full throttle, validation.
// The program is constantly waiting on events that might or might not get processed
// by the calibration fsm. The fsm navigates through the stages and calls the functions related to
// that stage of calibration after every transition. It also communicates with the user by logging
// on the screen.
// It is important that when moving on to the full brake or full throttle stages, the pedal is
// placed in that position before entering that stage.
#include "ads1015_def.h"
#include "crc32.h"
#include "debouncer.h"
#include "delay.h"
#include "gpio_it.h"
#include "input_event.h"
#include "interrupt.h"
#include "log.h"
#include "pedal_calibration.h"
#include "pedal_calibration_fsm.h"
#include "persist.h"
#include "test_helpers.h"
#include "unity.h"

static PedalCalibrationStorage s_storage;
static FSM s_fsm;
static Ads1015Storage s_ads1015_storage;
static ThrottleCalibrationData s_throttle_calibration_data;

// Inputs user might want to change:
#define TEST_PEDAL_CALIBRATION_ADC_CHANNEL_A ADS1015_CHANNEL_0
#define TEST_PEDAL_CALIBRATION_ADC_CHANNEL_B ADS1015_CHANNEL_1
#define TEST_PEDAL_CALIBRATION_BRAKE_PERCENTAGE 30
#define TEST_PEDAL_CALIBRATION_COAST_PERCENTAGE 10
#define TEST_PEDAL_CALIBRATION_TOLERANCE_SAFETY_FACTOR 2

static void prv_step_event(EventID event_id) {
  Event e = { .id = event_id };
  fsm_process_event(&s_fsm, &e);
}

void setup_test(void) {
  gpio_init();
  interrupt_init();
  gpio_it_init();
  soft_timer_init();

  I2CSettings i2c_settings = {
    .speed = I2C_SPEED_FAST,                    //
    .scl = { .port = GPIO_PORT_B, .pin = 10 },  //
    .sda = { .port = GPIO_PORT_B, .pin = 11 },  //
  };
  i2c_init(TEST_ADS1015_I2C_PORT, &i2c_settings);
  GPIOAddress ready_pin = {
    .port = GPIO_PORT_B,  //
    .pin = 2,             //
  };
  event_queue_init();

  PedalCalibrationSettings calib_settings = {
    .ads1015_storage = &s_ads1015_storage,
    .throttle_calibration_data = &s_throttle_calibration_data,
    .adc_channel = { TEST_PEDAL_CALIBRATION_ADC_CHANNEL_A, TEST_PEDAL_CALIBRATION_ADC_CHANNEL_B },
    .brake_zone_percentage = TEST_PEDAL_CALIBRATION_BRAKE_PERCENTAGE,
    .coast_zone_percentage = TEST_PEDAL_CALIBRATION_COAST_PERCENTAGE,
    .bounds_tolerance = 1
  };

  ads1015_init(&s_ads1015_storage, TEST_ADS1015_I2C_PORT, TEST_ADS1015_ADDR, &ready_pin);
  pedal_calibration_init(&s_storage, &calib_settings);
  pedal_calibration_fsm_init(&s_fsm, &s_storage);
}

void teardown_test(void) {}

void test_pedal_calibration(void) {
  Event e;
  LOG_DEBUG("Starting calibration\n");
  prv_step_event(INPUT_EVENT_PEDAL_CALIBRATION_FULL_BRAKE);
  LOG_DEBUG("Moving to full throttle stage.\n");
  delay_s(5);
  prv_step_event(INPUT_EVENT_PEDAL_CALIBRATION_FULL_THROTTLE);
  delay_s(5);
  prv_step_event(INPUT_EVENT_PEDAL_CALIBRATION_ENTER_VALIDATION);

  while (true) {
    if (status_ok(event_process(&e))) {
      fsm_process_event(&s_fsm, &e);
    }
  }
}
