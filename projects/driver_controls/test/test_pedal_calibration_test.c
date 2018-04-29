#include "ads1015_def.h"
#include "debouncer.h"
#include "delay.h"
#include "gpio_it.h"
#include "input_event.h"
#include "interrupt.h"
#include "log.h"
#include "pedal_calibration.h"
#include "pedal_calibration_fsm.h"
#include "persist.h"
#include "string.h"
#include "test_helpers.h"
#include "unity.h"

static PedalCalibrationStorage s_storage;
static Ads1015Storage s_ads1015_storage;
static ThrottleCalibrationData s_throttle_calibration_data;

// Inputs user might want to change:
#define TEST_PEDAL_CALIBRATION_ADC_CHANNEL_A ADS1015_CHANNEL_0
#define TEST_PEDAL_CALIBRATION_ADC_CHANNEL_B ADS1015_CHANNEL_1
#define TEST_PEDAL_CALIBRATION_BRAKE_PERCENTAGE 30
#define TEST_PEDAL_CALIBRATION_COAST_PERCENTAGE 30
#define TEST_PEDAL_CALIBRATION_TOELRANCE_SAFETY_FACTOR 2

// The number of mocked readings for a state and a channel.
#define TEST_PEDAL_CALIBRATION_NUM_MOCKED_READINGS 4

// Counts every time both channels are read in a state.
static uint8_t s_sample_counter[NUM_PEDAL_CALIBRATION_CHANNELS];

// The array contains mocked readings for all channels and states.
static int16_t s_mocked_reading[NUM_PEDAL_CALIBRATION_STATES][NUM_PEDAL_CALIBRATION_CHANNELS]
                               [TEST_PEDAL_CALIBRATION_NUM_MOCKED_READINGS] = {
                                 { { 600, 620, 630, 570 }, { 80, 120, 100, 90 } },
                                 { { 1175, 1180, 1200, 1225 }, { 500, 470, 530, 510 } }
                               };

// The current state provided to the mocked ads1015_read_raw.
static PedalCalibrationState s_current_state;

// Mocks ads1015_read_raw. Reads from s_mocked_reading.
StatusCode TEST_MOCK(ads1015_read_raw)(Ads1015Storage *storage, Ads1015Channel channel,
                                       int16_t *reading) {
  if (channel == TEST_PEDAL_CALIBRATION_ADC_CHANNEL_A) {
    uint8_t nth_sample =
        s_sample_counter[PEDAL_CALIBRATION_CHANNEL_A] % TEST_PEDAL_CALIBRATION_NUM_MOCKED_READINGS;
    *reading = s_mocked_reading[s_current_state][PEDAL_CALIBRATION_CHANNEL_A][nth_sample];
    s_sample_counter[PEDAL_CALIBRATION_CHANNEL_A]++;

  } else {
    uint8_t nth_sample =
        s_sample_counter[PEDAL_CALIBRATION_CHANNEL_B] % TEST_PEDAL_CALIBRATION_NUM_MOCKED_READINGS;
    *reading = s_mocked_reading[s_current_state][PEDAL_CALIBRATION_CHANNEL_B]
                               [s_sample_counter[PEDAL_CALIBRATION_CHANNEL_B] %
                                TEST_PEDAL_CALIBRATION_NUM_MOCKED_READINGS];
    s_sample_counter[PEDAL_CALIBRATION_CHANNEL_B]++;
  }

  return STATUS_CODE_OK;
}

static bool prv_calibration_correct(ThrottleCalibrationData *result,
                                    ThrottleCalibrationData *expected) {
  bool equal = true;
  for (ThrottleZone zone = THROTTLE_ZONE_BRAKE; zone < NUM_THROTTLE_ZONES; zone++) {
    equal &= (result->zone_thresholds_main[zone].min == expected->zone_thresholds_main[zone].min) &&
             (result->zone_thresholds_main[zone].max == expected->zone_thresholds_main[zone].max);
    LOG_DEBUG("zone thresholds: min: %d == %d - max: %d == %d\n",
              result->zone_thresholds_main[zone].min, expected->zone_thresholds_main[zone].min,
              result->zone_thresholds_main[zone].max, expected->zone_thresholds_main[zone].max);
  }
  for (ThrottleChannel channel = THROTTLE_CHANNEL_MAIN; channel < NUM_THROTTLE_CHANNELS;
       channel++) {
    equal &=
        (result->line[channel].full_brake_reading == expected->line[channel].full_brake_reading) &&
        (result->line[channel].full_throttle_reading ==
         expected->line[channel].full_throttle_reading);
    LOG_DEBUG("line: brake: %d == %d - throttle: %d == %d\n",
              result->line[channel].full_brake_reading, expected->line[channel].full_brake_reading,
              result->line[channel].full_throttle_reading,
              expected->line[channel].full_throttle_reading);
  }
  equal &= result->tolerance == expected->tolerance;
  LOG_DEBUG("tolerance: %d == %d\n", result->tolerance, expected->tolerance);
  equal &= result->channel_main == expected->channel_main;
  LOG_DEBUG("channel main: %d == %d\n", result->channel_main, expected->channel_main);
  equal &= result->channel_secondary == expected->channel_secondary;
  LOG_DEBUG("channel secondary: %d == %d\n", result->channel_secondary,
            expected->channel_secondary);
  return equal;
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
  ads1015_init(&s_ads1015_storage, TEST_ADS1015_I2C_PORT, TEST_ADS1015_ADDR, &ready_pin);
  pedal_calibration_init(
      &s_storage, &s_ads1015_storage, TEST_PEDAL_CALIBRATION_ADC_CHANNEL_A,
      TEST_PEDAL_CALIBRATION_ADC_CHANNEL_B, TEST_PEDAL_CALIBRATION_BRAKE_PERCENTAGE,
      TEST_PEDAL_CALIBRATION_COAST_PERCENTAGE, TEST_PEDAL_CALIBRATION_TOELRANCE_SAFETY_FACTOR);
}

void teardown_test(void) {}

void test_pedal_calibration_correctness(void) {
  memset(s_sample_counter, 0, sizeof(s_sample_counter));
  s_current_state = PEDAL_CALIBRATION_STATE_FULL_BRAKE;
  pedal_calibration_process_state(&s_storage, PEDAL_CALIBRATION_STATE_FULL_BRAKE);

  memset(s_sample_counter, 0, sizeof(s_sample_counter));
  s_current_state = PEDAL_CALIBRATION_STATE_FULL_THROTTLE;
  pedal_calibration_process_state(&s_storage, PEDAL_CALIBRATION_STATE_FULL_THROTTLE);

  pedal_calibration_calculate(&s_storage, &s_throttle_calibration_data);
  ThrottleCalibrationData expected = {
    .zone_thresholds_main[THROTTLE_ZONE_BRAKE] = { .min = 570, .max = 766 },
    .zone_thresholds_main[THROTTLE_ZONE_COAST] = { .min = 767, .max = 963 },
    .zone_thresholds_main[THROTTLE_ZONE_ACCEL] = { .min = 964, .max = 1225 },
    .line[THROTTLE_CHANNEL_MAIN] = { .full_brake_reading = 600, .full_throttle_reading = 1200 },
    .line[THROTTLE_CHANNEL_SECONDARY] = { .full_brake_reading = 100, .full_throttle_reading = 500 },
    .tolerance = 60,
    .channel_main = TEST_PEDAL_CALIBRATION_ADC_CHANNEL_A,
    .channel_secondary = TEST_PEDAL_CALIBRATION_ADC_CHANNEL_B
  };
  TEST_ASSERT_TRUE(prv_calibration_correct(&s_throttle_calibration_data, &expected));
}
