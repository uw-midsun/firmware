#pragma once

typedef struct MagneticCalibrationData {
  int16_t reading;
  uint16_t percentage;
} MagneticCalibrationData;

typedef struct MagneticBrakeSettings {
  int16_t percentage_threshold;
  uint16_t zero_value;
  uint16_t hundred_value;
  uint16_t min_allowed_range;
  uint16_t max_allowed_range;
} MagneticBrakeSettings;

StatusCode magnetic_brake_event_generator_init(MagneticCalibrationData *data,
                                               MagneticBrakeSettings *brake_settings);
StatusCode percentage_converter(MagneticCalibrationData *data,
                                MagneticBrakeSettings *brake_settings);
