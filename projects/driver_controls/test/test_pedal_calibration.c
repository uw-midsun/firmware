#include "ads1015_def.h"
#include "delay.h"
#include "gpio_it.h"
#include "input_event.h"
#include "interrupt.h"
#include "pedal_calibration.h"
#include "test_helpers.h"
#include "unity.h"

static PedalCalibrationStorage s_storage;
static Ads1015Storage s_ads1015_storage;
#define TEST_PEDAL_CALIBRATION_ADC_CHANNEL_A ADS1015_CHANNEL_0
#define TEST_PEDAL_CALIBRATION_ADC_CHANNEL_B ADS1015_CHANNEL_1

void setup_test(void){
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
  ads1015_init(&s_ads1015_storage, TEST_ADS1015_I2C_PORT, TEST_ADS1015_ADDR, &ready_pin);
  pedal_calibration_init(&s_storage, &s_ads1015_storage, TEST_PEDAL_CALIBRATION_ADC_CHANNEL_A,
                         TEST_PEDAL_CALIBRATION_ADC_CHANNEL_B);
}

void test_pedal_calibration_start(){
  pedal_calibration_get_band(&s_storage, PEDAL_CALIBRATION_STATE_FULL_BRAKE);
  pedal_calibration_get_band(&s_storage, PEDAL_CALIBRATION_STATE_FULL_THROTTLE);
}

void teardown_test(void){}