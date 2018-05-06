// The goal of calibration is to provide an initialized ThrottleCalibrationData storage for the
// throttle module. This structure contains:
//    1. A main and a secondary channel.
//    2. Two lines that describe the voltage-position graph of the pedal for both channels.
//    3. A tolerance around the secondary line.
//    4. Zone thresholds that divide the pedal range of motion into 3 zones.
// Calibration starts by pedal_calibration_init.
// The next phase is to collect data from the pedal.
// Pedal is kept at a fixed position, full brake/throttle, for pedal_calibration_process_state
// to find how much the readings could vary in the same position.
// pedal_calibration_calculate uses this info to initialize ThrottleCalibrationData.
// It first finds the main channel (hi-res) and secondary channel (lo-res).
// Note that varying of data creates a band rather than a line for each channel. For the main
// channel we need a line as our source that "maps" a voltage (coming from main channel) to a
// position. On the second channel we need a band that maps that position to a range of possible
// readings (on second channel) which hopefully contains the actual secondary reading.
// That is how we make sure channel readings agree.
// The lines are set as the midline of their bands, and the range (tolerance) is half the second
// band's width times a safety factor. The zone thresholds are also calculated by the input zone
// percentages.
#include "pedal_calibration.h"
#include "critical_section.h"
#include "string.h"
#include "throttle.h"

#define PEDAL_CALIBRATION_NUM_SAMPLES 1000

// ADS1015 user callback: it is called after every new conversion and reads raw values and
// updates the thresholds of the band for the current state and the calling channel.
// It will be called PEDAL_CALIBRATION_NUM_SAMPLES number of times.
static void prv_adc_callback(Ads1015Channel ads1015_channel, void *context) {
  PedalCalibrationStorage *storage = context;
  PedalCalibrationChannel channel = NUM_PEDAL_CALIBRATION_CHANNELS;
  PedalCalibrationState state = storage->state;
  int16_t reading = 0;

  // Find out which channel corresponds to the ads1015_channel.
  for (channel = PEDAL_CALIBRATION_CHANNEL_A; channel < NUM_PEDAL_CALIBRATION_CHANNELS; channel++) {
    if (storage->settings.adc_channel[channel] == ads1015_channel) {
      break;
    }
  }

  // Check if there are enough samples taken for the channel.
  if (storage->sample_counter[channel] >= PEDAL_CALIBRATION_NUM_SAMPLES) {
    // If yes, stop sampling and remove the callback.
    ads1015_configure_channel(storage->settings.ads1015_storage, ads1015_channel, false, NULL,
                              NULL);
  } else {
    // If not, continue reading.
    ads1015_read_raw(storage->settings.ads1015_storage, ads1015_channel, &reading);
    storage->sample_counter[channel]++;

    PedalCalibrationRange *range = &storage->band[channel][state];
    range->min = MIN(range->min, reading);
    range->max = MAX(range->max, reading);
  }
}

// Initializes the calibration storage. Requires initialized Ads1015storage.
StatusCode pedal_calibration_init(PedalCalibrationStorage *storage,
                                  PedalCalibrationSettings *settings) {
  // TODO: validate inputs
  memset(storage, 0, sizeof(*storage));
  storage->settings = *settings;

  return STATUS_CODE_OK;
}

// Given the state (full throttle/brake), it finds the min and max raw readings of each channel.
// These points would become the vertices of the band we want to obtain around the data.
StatusCode pedal_calibration_process_state(PedalCalibrationStorage *storage,
                                           PedalCalibrationState state) {
  if (storage == NULL || state >= NUM_PEDAL_CALIBRATION_STATES) {
    return status_code(STATUS_CODE_INVALID_ARGS);
  }

  bool disabled = critical_section_start();
  storage->state = state;
  storage->sample_counter[PEDAL_CALIBRATION_CHANNEL_A] = 0;
  storage->sample_counter[PEDAL_CALIBRATION_CHANNEL_B] = 0;

  int16_t reading;
  for (PedalCalibrationChannel channel = PEDAL_CALIBRATION_CHANNEL_A;
       channel < NUM_PEDAL_CALIBRATION_CHANNELS; channel++) {
    storage->band[channel][state].min = INT16_MAX;
    storage->band[channel][state].max = INT16_MIN;
  }
  critical_section_end(disabled);

  // Set up callback on channels which would read new values after each conversion.
  // Since we disable the channel reads as we obtain enough samples, they need to be reconfigured
  // for the new stage.
  status_ok_or_return(ads1015_configure_channel(
      storage->settings.ads1015_storage, storage->settings.adc_channel[PEDAL_CALIBRATION_CHANNEL_A],
      true, prv_adc_callback, storage));
  status_ok_or_return(ads1015_configure_channel(
      storage->settings.ads1015_storage, storage->settings.adc_channel[PEDAL_CALIBRATION_CHANNEL_B],
      true, prv_adc_callback, storage));

  // Wait until both channels finish sampling.
  while (storage->sample_counter[PEDAL_CALIBRATION_CHANNEL_A] < PEDAL_CALIBRATION_NUM_SAMPLES &&
         storage->sample_counter[PEDAL_CALIBRATION_CHANNEL_B] < PEDAL_CALIBRATION_NUM_SAMPLES) {
  }

  return STATUS_CODE_OK;
}

static void prv_calc_midline(PedalCalibrationStorage *storage,
                             PedalCalibrationChannel calib_channel,
                             ThrottleChannel throttle_channel) {
  PedalCalibrationRange *range = storage->band[calib_channel];
  storage->settings.throttle_calibration_data->line[throttle_channel] = (ThrottleLine){
    .full_brake_reading = (range[PEDAL_CALIBRATION_STATE_FULL_BRAKE].max +
                           range[PEDAL_CALIBRATION_STATE_FULL_BRAKE].min) /
                          2,
    .full_throttle_reading = (range[PEDAL_CALIBRATION_STATE_FULL_THROTTLE].max +
                              range[PEDAL_CALIBRATION_STATE_FULL_THROTTLE].min) /
                             2,
  };
}

static int16_t prv_calc_range(PedalCalibrationRange *range) {
  return range[PEDAL_CALIBRATION_STATE_FULL_THROTTLE].max -
         range[PEDAL_CALIBRATION_STATE_FULL_BRAKE].min;
}

// Based on the points initialized in the bands, it calculates and initializes the data in
// ThrottleCalibrationData.
StatusCode pedal_calibration_calculate(PedalCalibrationStorage *storage) {
  if (storage == NULL) {
    return status_code(STATUS_CODE_INVALID_ARGS);
  }

  ThrottleCalibrationData *throttle_calibration = storage->settings.throttle_calibration_data;

  // Compare the range of bands from channels and determine the channels as main or secondary.
  int16_t range_a = prv_calc_range(storage->band[PEDAL_CALIBRATION_CHANNEL_A]);
  int16_t range_b = prv_calc_range(storage->band[PEDAL_CALIBRATION_CHANNEL_B]);

  PedalCalibrationChannel main_channel = PEDAL_CALIBRATION_CHANNEL_A;
  PedalCalibrationChannel secondary_channel = PEDAL_CALIBRATION_CHANNEL_B;

  if (range_a < range_b) {
    main_channel = PEDAL_CALIBRATION_CHANNEL_B;
    secondary_channel = PEDAL_CALIBRATION_CHANNEL_A;
  }

  throttle_calibration->channel_main = storage->settings.adc_channel[main_channel];
  throttle_calibration->channel_secondary = storage->settings.adc_channel[secondary_channel];

  // Get the main band's vertices.
  prv_calc_midline(storage, main_channel, THROTTLE_CHANNEL_MAIN);
  // Get the secondary band's vertices.
  prv_calc_midline(storage, secondary_channel, THROTTLE_CHANNEL_SECONDARY);

  // Get the full range for main channel so we can calculate the size of each zone.
  int16_t range = prv_calc_range(storage->band[main_channel]);

  // Use the percentages to calculate the thresholds of each zone.
  ThrottleZoneThreshold *brake_threshold =
                            &throttle_calibration->zone_thresholds_main[THROTTLE_ZONE_BRAKE],
                        *coast_threshold =
                            &throttle_calibration->zone_thresholds_main[THROTTLE_ZONE_COAST],
                        *accel_threshold =
                            &throttle_calibration->zone_thresholds_main[THROTTLE_ZONE_ACCEL];
  PedalCalibrationRange *calib_range = storage->band[main_channel];

  brake_threshold->min = calib_range[PEDAL_CALIBRATION_STATE_FULL_BRAKE].min;
  brake_threshold->max =
      brake_threshold->min + (storage->settings.brake_zone_percentage * range / 100);
  int16_t brake_tolerance =
      (brake_threshold->max - brake_threshold->min) * storage->settings.bounds_tolerance / 100;
  brake_threshold->min -= brake_tolerance;

  coast_threshold->min = brake_threshold->max + 1;
  coast_threshold->max =
      coast_threshold->min + (storage->settings.coast_zone_percentage * range / 100);

  accel_threshold->min = coast_threshold->max + 1;
  accel_threshold->max = calib_range[PEDAL_CALIBRATION_STATE_FULL_THROTTLE].max;
  int16_t accel_tolerance =
      (accel_threshold->max - accel_threshold->min) * storage->settings.bounds_tolerance / 100;
  accel_threshold->max += accel_tolerance;

  // Tolerance should be half of the band's width assuming the width is constant.
  // In this case we take the maximum of widths at both ends multiplied by the given safety factor.
  // TODO: this math is wrong - hardcoded value for now
  throttle_calibration->tolerance = 20;

  return STATUS_CODE_OK;
}
