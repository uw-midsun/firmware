#include "steering_wheel.h"
#include "steering_wheel_calibration.h"
#include <string.h>

static SteeringWheelCalibrationData wheel_calib_data;
static SteeringWheelStorage s_steering_wheel_storage;

static void prv_set_calibration_data (SteeringWheelCalibrationData *calib_data) {
  calib_data->min_bound = 0;
  calib_data->max_bound = 4000;
  calib_data->range = 4000;
  calib_data->midpoint = 2000;
}

void setup_test(void) {
  gpio_init();
  interrupt_init();
  soft_timer_init();

  ADCChannel conversion_channel;
  const GPIOAddress conversion_address = {
    .port = GPIO_PORT_A,
    .pin = 7,
  };

  adc_init(ADC_MODE_CONTINUOUS);
  adc_get_channel(conversion_address, &conversion_channel);
  adc_set_channel(conversion_channel, true);

  prv_set_calibration_data(wheel_calib_data);
  steering_wheel_init(s_steering_wheel_storage, wheel_calib_data);
}
