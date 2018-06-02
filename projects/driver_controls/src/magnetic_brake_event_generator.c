#include <stdint.h>
#include <stdio.h>

#include <stdbool.h>
#include "adc.h"
#include "ads1015.h"
#include "ads1015_def.h"
#include "delay.h"
#include "event_arbiter.h"
#include "gpio_it.h"
#include "i2c.h"
#include "input_event.h"
#include "interrupt.h"
#include "log.h"
#include "soft_timer.h"
#include "unity.h"
#include "event_queue.h"
#include "magnetic_brake_event_generator.h"
#include "status.h"

static int16_t percentage_converter(int16_t reading_in_lsb, MagneticSensorReading *reading, MagneticBrakeSettings *brake_settings) {

  int16_t percentage;

  if (reading->zero_value > reading->hundred_value) {
    percentage = ((100 * (reading_in_lsb - reading->hundred_value)) / (reading->hundred_value - reading->zero_value)) + 100;
  } else {
    percentage = (100 * (reading_in_lsb - reading->zero_value)) / (reading->hundred_value - reading->zero_value);
  }

  if (percentage < 0) {
    percentage = 0;
  } else if (percentage > 100) {
    percentage = 100;
  }

  if(percentage < reading->min_allowed_range || percentage > reading->max_allowed_range){
  	printf("%s\n", "cannot perform calibration, try again" );
  	return -1;
  }

  if(percentage > reading->brake_settings_threshold){
  	event_raise(INPUT_EVENT_MECHANICAL_BRAKE_PRESSED, percentage);
  }

  else{
  	event_raise(INPUT_EVENT_MECHANICAL_BRAKE_PRESSED, percentage);
  }

  return percentage;
}

static void input_values(MagneticCalibrationData *data, MagneticSensorReading *reading, MagneticBrakeSettings *brake_settings){

	printf("%s\n", "Brake sensor is calibrating, Please ensure the brake is not being pressed, wait for response to continue");
	int16_t percentage = -1;

	while(percentage = -1){
		percentage = percentage_converter(data->reading, reading-> zero_value, brake_settings);
	}

	// reading->zero_value = percentage;
	// printf("%s\n","Initial calibration complete, Please press and hold the brake for aprroximately 4 seconds, wait for response to continue");

	// int16_t percentage = -1;

	// while(percentage = -1){
	// 	percentage = percentage_converter(data->reading, reading-> zero_value, brake_settings);
	// }

	// reading->hundred_value = percentage;
	printf("%s\n","Final calibration complete." );

}

StatusCode magnetic_brake_event_generator_init(MagneticSensorReading *reading, MagneticCalibrationData *data, MagneticBrakeSettings *brake_settings){

	input_values(data,reading,brake_settings);

	return STATUS_CODE_OK;
}





