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
#include "string.h"
#include "throttle.h"

#define PEDAL_CALIBRATION_NUM_SAMPLES 100

// Initializes the calibration storage. Requires initialized Ads1015storage.
StatusCode pedal_calibration_init(PedalCalibrationStorage *storage, Ads1015Storage *ads1015_storage,
                                  Ads1015Channel channel_a, Ads1015Channel channel_b,
                                  uint8_t brake_zone_percentage, uint8_t coast_zone_percentage,
                                  uint8_t tolerance_safety_factor) {
  if (storage == NULL || ads1015_storage == NULL || channel_a >= NUM_ADS1015_CHANNELS ||
      channel_b >= NUM_ADS1015_CHANNELS || (brake_zone_percentage + coast_zone_percentage) >= 100) {
    return status_code(STATUS_CODE_INVALID_ARGS);
  }
  memset(storage, 0, sizeof(*storage));
  storage->ads1015_storage = ads1015_storage;
  storage->adc_channel[PEDAL_CALIBRATION_CHANNEL_A] = channel_a;
  storage->adc_channel[PEDAL_CALIBRATION_CHANNEL_B] = channel_b;
  storage->brake_percentage = brake_zone_percentage;
  storage->coast_percentage = coast_zone_percentage;
  storage->tolerance_safety_factor = tolerance_safety_factor;
  return STATUS_CODE_OK;
}

// ADS1015 user callback: it is called after every new conversion and reads raw values and
// updates the thresholds of the band for the current state and the calling channel.
// It will be called PEDAL_CALIBRATION_NUM_SAMPLES number of times.
void pedal_calibration_adc_callback(Ads1015Channel ads1015_channel, void *context) {
  PedalCalibrationStorage *storage = context;
  PedalCalibrationChannel channel;
  PedalCalibrationState state = storage->state;
  int16_t reading;

  // Find out which channel corresponds to the ads1015_channel.
  for (channel = PEDAL_CALIBRATION_CHANNEL_A; channel < NUM_PEDAL_CALIBRATION_CHANNELS; channel++) {
    if (storage->adc_channel[channel] == ads1015_channel) {
      break;
    }
  }
  // Check if there are enough samples taken for the channel.
  if (storage->sample_counter[channel] >= PEDAL_CALIBRATION_NUM_SAMPLES) {
    // If yes, stop sampling and remove the callback.
    ads1015_configure_channel(storage->ads1015_storage, ads1015_channel, true, NULL, NULL);
  } else {
    // If not, continue reading.
    ads1015_read_raw(storage->ads1015_storage, ads1015_channel, &reading);
    storage->sample_counter[channel]++;
    // Update the min and max reading if needed.
    if (reading < storage->band[channel][state].min) {
      storage->band[channel][state].min = reading;
    } else if (reading > storage->band[channel][state].max) {
      storage->band[channel][state].max = reading;
    }
  }
}

// Given the state (full throttle/brake), it finds the min and max raw readings of each channel.
// These points would become the vertices of the band we want to obtain around the data.
StatusCode pedal_calibration_process_state(PedalCalibrationStorage *storage,
                                           PedalCalibrationState state) {
  if (storage == NULL || state >= NUM_PEDAL_CALIBRATION_STATES) {
    return status_code(STATUS_CODE_INVALID_ARGS);
  }
  storage->state = state;
  storage->sample_counter[PEDAL_CALIBRATION_CHANNEL_A] = 0;
  storage->sample_counter[PEDAL_CALIBRATION_CHANNEL_B] = 0;

  status_ok_or_return(ads1015_configure_channel(storage->ads1015_storage,
                                                storage->adc_channel[PEDAL_CALIBRATION_CHANNEL_A],
                                                true, NULL, NULL));
  status_ok_or_return(ads1015_configure_channel(storage->ads1015_storage,
                                                storage->adc_channel[PEDAL_CALIBRATION_CHANNEL_B],
                                                true, NULL, NULL));
  int16_t reading;
  for (PedalCalibrationChannel channel = PEDAL_CALIBRATION_CHANNEL_A;
       channel < NUM_PEDAL_CALIBRATION_CHANNELS; channel++) {
    // Read once first to initialize the max and min.
    status_ok_or_return(
        ads1015_read_raw(storage->ads1015_storage, storage->adc_channel[channel], &reading));
    storage->band[channel][state].min = reading;
    storage->band[channel][state].max = reading;
  }
  // Set up callback on channels which would read new values after each conversion.
  status_ok_or_return(ads1015_configure_channel(storage->ads1015_storage,
                                                storage->adc_channel[PEDAL_CALIBRATION_CHANNEL_A],
                                                true, pedal_calibration_adc_callback, storage));
  status_ok_or_return(ads1015_configure_channel(storage->ads1015_storage,
                                                storage->adc_channel[PEDAL_CALIBRATION_CHANNEL_B],
                                                true, pedal_calibration_adc_callback, storage));
  // Wait until both channels finish sampling.
  while (storage->sample_counter[PEDAL_CALIBRATION_CHANNEL_A] < PEDAL_CALIBRATION_NUM_SAMPLES ||
         storage->sample_counter[PEDAL_CALIBRATION_CHANNEL_B] < PEDAL_CALIBRATION_NUM_SAMPLES) {
  }
  return STATUS_CODE_OK;
}

// Based on the points initialized in the bands, it calculates and initiliazes the data in
// ThrottleCalibrationData.
StatusCode pedal_calibration_calculate(PedalCalibrationStorage *storage,
                                       ThrottleCalibrationData *throttle_calibration) {
  if (storage == NULL || throttle_calibration == NULL) {
    return status_code(STATUS_CODE_INVALID_ARGS);
  }
  // Compare the range of bands from channels and determine the channels as main or secondary.
  int16_t range_a =
      storage->band[PEDAL_CALIBRATION_CHANNEL_A][PEDAL_CALIBRATION_STATE_FULL_THROTTLE].max -
      storage->band[PEDAL_CALIBRATION_CHANNEL_A][PEDAL_CALIBRATION_STATE_FULL_BRAKE].min;
  int16_t range_b =
      storage->band[PEDAL_CALIBRATION_CHANNEL_B][PEDAL_CALIBRATION_STATE_FULL_THROTTLE].max -
      storage->band[PEDAL_CALIBRATION_CHANNEL_B][PEDAL_CALIBRATION_STATE_FULL_BRAKE].min;

  PedalCalibrationChannel main_channel = PEDAL_CALIBRATION_CHANNEL_A;
  PedalCalibrationChannel secondary_channel = PEDAL_CALIBRATION_CHANNEL_B;

  if (range_a < range_b) {
    main_channel = PEDAL_CALIBRATION_CHANNEL_B;
    secondary_channel = PEDAL_CALIBRATION_CHANNEL_A;
  }

  throttle_calibration->channel_main = storage->adc_channel[main_channel];
  throttle_calibration->channel_secondary = storage->adc_channel[secondary_channel];

  // Get the main band's vertices.
  int16_t min_brake = storage->band[main_channel][PEDAL_CALIBRATION_STATE_FULL_BRAKE].min;

  int16_t max_brake = storage->band[main_channel][PEDAL_CALIBRATION_STATE_FULL_BRAKE].max;

  int16_t min_accel = storage->band[main_channel][PEDAL_CALIBRATION_STATE_FULL_THROTTLE].min;

  int16_t max_accel = storage->band[main_channel][PEDAL_CALIBRATION_STATE_FULL_THROTTLE].max;

  // Calculate the main line as a "midline" of the main band.
  throttle_calibration->line[THROTTLE_CHANNEL_MAIN].full_brake_reading =
      (min_brake + max_brake) / 2;
  throttle_calibration->line[THROTTLE_CHANNEL_MAIN].full_throttle_reading =
      (min_accel + max_accel) / 2;

  // Get the full range for main channel. Range is useful since when multiplied by zone percentages,
  // it will give the size of each zone.
  int16_t range = storage->band[main_channel][PEDAL_CALIBRATION_STATE_FULL_THROTTLE].max -
                  storage->band[main_channel][PEDAL_CALIBRATION_STATE_FULL_BRAKE].min;

  // Use the percentages to calculate the thresholds of each zone.
  ThrottleZoneThreshold *zone_thresholds = throttle_calibration->zone_thresholds_main;

  zone_thresholds[THROTTLE_ZONE_BRAKE].min = min_brake;
  zone_thresholds[THROTTLE_ZONE_BRAKE].max =
      zone_thresholds[THROTTLE_ZONE_BRAKE].min + (storage->brake_percentage * range / 100);

  zone_thresholds[THROTTLE_ZONE_COAST].min = zone_thresholds[THROTTLE_ZONE_BRAKE].max + 1;
  zone_thresholds[THROTTLE_ZONE_COAST].max =
      zone_thresholds[THROTTLE_ZONE_COAST].min + (storage->coast_percentage * range / 100);

  zone_thresholds[THROTTLE_ZONE_ACCEL].min = zone_thresholds[THROTTLE_ZONE_COAST].max + 1;
  zone_thresholds[THROTTLE_ZONE_ACCEL].max = max_accel;

  // Get the secondary band's vertices.
  min_brake = storage->band[secondary_channel][PEDAL_CALIBRATION_STATE_FULL_BRAKE].min;

  max_brake = storage->band[secondary_channel][PEDAL_CALIBRATION_STATE_FULL_BRAKE].max;

  min_accel = storage->band[secondary_channel][PEDAL_CALIBRATION_STATE_FULL_THROTTLE].min;

  max_accel = storage->band[secondary_channel][PEDAL_CALIBRATION_STATE_FULL_THROTTLE].max;

  // Calculate the secondary line as a "midline" of the secondary band.
  throttle_calibration->line[THROTTLE_CHANNEL_SECONDARY].full_brake_reading =
      (min_brake + max_brake) / 2;
  throttle_calibration->line[THROTTLE_CHANNEL_SECONDARY].full_throttle_reading =
      (min_accel + max_accel) / 2;
  // Tolerance should be half of the band's width assuming the width is constant.
  // In this case we take the maximum of widths at both ends multiplied by the given safety factor.
  throttle_calibration->tolerance =
      MAX((max_brake - min_brake) / 2, (max_accel - min_accel) / 2) * storage->tolerance_safety_factor;

  return STATUS_CODE_OK;
}
