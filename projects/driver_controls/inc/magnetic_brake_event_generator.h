#pragma once

typedef struct MagneticCalibrationData{
	int16_t reading;
	Ads1015Storage storage;
} MagneticCalibrationData;

typedef struct MagneticBrakeSettings{
	int16_t percentage_threshold;
} MagneticBrakeSettings;

typedef struct MagneticSensorReading{
	int16_t zero_value;
	int16_t hundred_value;
	int16_t min_allowed_range;
	int16_t max_allowed_range;
} MagneticSensorReading;

StatusCode magnetic_brake_event_generator_init(MagneticSensorReading *reading, MagneticCalibrationData *data, MagneticBrakeSettings *brake_settings);


