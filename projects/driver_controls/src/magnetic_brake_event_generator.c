#include <stdint.h>
#include <stdio.h>
#include <limits.h>

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


 StatusCode percentage_converter(MagneticCalibrationData *data, MagneticBrakeSettings *brake_settings) {

  uint16_t percentage;

  if (brake_settings->zero_value > brake_settings->hundred_value) {
    percentage = ((brake_settings->min_allowed_range * (data->reading - brake_settings->hundred_value)) / (brake_settings->hundred_value - brake_settings->zero_value)) + brake_settings->max_allowed_range;
  } else {
    percentage = (brake_settings->max_allowed_range * (data->reading - brake_settings->zero_value)) / (brake_settings->hundred_value - brake_settings->zero_value);
  }

  //add code to see if the percentage is greater than max allowed value 
  if(percentage > brake_settings->percentage_threshold){
  	event_raise(INPUT_EVENT_MECHANICAL_BRAKE_PRESSED, percentage);
  } else{
  	event_raise(INPUT_EVENT_MECHANICAL_BRAKE_PRESSED, percentage);
  }

  data->percentage = percentage;

  return STATUS_CODE_OK;
}

static void input_values(MagneticCalibrationData *data, MagneticBrakeSettings *brake_settings){

	uint16_t first_samples[1000];
	uint16_t second_samples[1000];
	uint16_t temp1, temp2, temp3, temp4;

	
	printf("%s\n","Brake sensor is calibrating, Please ensure the brake is not being pressed, wait for response to continue" );

	for(int i = 0; i < 1000; i++){
		percentage_converter(data, brake_settings);
		first_samples[i] = data->percentage;
	}

	//max in range of samples = temp 1 and 3
	

	//min in range of samples = temp 2 and 4
	

	for(int i  = 0; i < 1000; i++){
		uint16_t max= 0;
		uint16_t min = brake_settings->max_allowed_range;

		temp1 = first_samples[i];
		temp2 = first_samples[i];

		if(temp1 > max){
			max = temp1;
		}

		if(temp2 <min){
			min = temp2;
		}
	}

	uint16_t average_lowest = (temp1 + temp2)/2;
	//reading->zero_value = average_lowest;

	printf("%s\n","Initial calibration complete, Please press and hold the brake for aprroximately 3 seconds, wait for response to continue");

	for(int i  = 0 ; i < 1000; i++){
		percentage_converter(data,brake_settings);
		second_samples[i] = data->percentage;
	}

	for(int i  = 0; i < 1000; i++){
		uint16_t max= 0;
		uint16_t min = brake_settings->max_allowed_range;


		temp3 = second_samples[i];
		temp4 = second_samples[i];

		if(temp3 > max){
			max = temp1;
		}

		if(temp4 <min){
			min = temp2;
		}
	}

	uint16_t average_highest = (temp3 + temp4)/2;
	//reading->hundred_value = average_highest;

	printf("%s\n","Final calibration complete." );
}

StatusCode magnetic_brake_event_generator_init(MagneticCalibrationData *data, MagneticBrakeSettings *brake_settings){

	input_values(data,brake_settings);

	return STATUS_CODE_OK;
}





