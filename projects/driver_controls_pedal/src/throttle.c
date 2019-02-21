// The module operates based on the voltage readings that come from the pedals.
// The pedal occupies two channels of the ADS1015. The channel with a higher resolution is chosen
// to be the "main" channel.
//
// A periodic safety check, prv_raise_event_timer_callback, checks for this flag and if ok, it
// updates the position of the pedal in storage and raises the events related to the pedal's
// position (braking, coasting, and accelerating zones).
//
// In this module, it is assumed that channels hold a linear relationship with respect to the
// pedal's position. This is used to verify if the readings are valid and "real" i.e if channels
// are synced. If the data turns out to be stale or channels aren't synced, a timeout event will
// be raised. The throttle_get_position function simply reads the position from storage.
#include "throttle.h"
#include <string.h>
#include "critical_section.h"
#include "event_queue.h"
#include "pc_input_event.h"
#include "log.h"

static ThrottleStorage s_throttle_storage;

// Scales a reading to a measure out of THROTTLE_DENOMINATOR based on the given range(min to max).
static uint16_t prv_scale_reading(int16_t reading, int16_t max, int16_t min) {
  return (uint16_t)(THROTTLE_DENOMINATOR * (reading - min) / (max - min));
}

// Given a reading from main channel and a zone, finds how far within that zone the pedal is pushed.
// The scale goes from 0(0%) to THROTTLE_DENOMINATOR(100%).
static uint16_t prv_get_numerator_zone(int16_t reading_main, ThrottleZone zone,
                                       ThrottleStorage *storage) {
  ThrottleZoneThreshold *zone_thresholds_main = storage->calibration_data->zone_thresholds_main;
  if (zone == THROTTLE_ZONE_BRAKE) {
    // Brake is at its max when pedal not pressed.
    return THROTTLE_DENOMINATOR - (prv_scale_reading(reading_main, zone_thresholds_main[zone].max,
                                                     zone_thresholds_main[zone].min));
  } else if (zone == THROTTLE_ZONE_COAST) {
    // Coast is either on or off, not a range of values.
    return THROTTLE_DENOMINATOR;
  } else {
    // Acceleration is at its max when pedal pressed fully.
    return prv_scale_reading(reading_main, zone_thresholds_main[zone].max,
                             zone_thresholds_main[zone].min);
  }
}

// Given a reading from main channel and a zone, finds if the reading belongs to that zone.
static bool prv_reading_within_zone(int16_t reading_main, ThrottleZone zone,
                                    ThrottleStorage *storage) {
  ThrottleZoneThreshold *zone_thresholds_main = storage->calibration_data->zone_thresholds_main;
  return ((reading_main <= zone_thresholds_main[zone].max) &&
          (reading_main >= zone_thresholds_main[zone].min));
}

// Returns true if the channel readings follow their linear relationship.
static bool prv_channels_synced(int16_t reading_main, int16_t reading_secondary,
                                ThrottleStorage *storage) {
  ThrottleZoneThreshold *zone_thresholds_main = storage->calibration_data->zone_thresholds_main;
  // Check if the reading_main is out of bounds.
  if ((reading_main < zone_thresholds_main[THROTTLE_ZONE_BRAKE].min) ||
      (reading_main > zone_thresholds_main[THROTTLE_ZONE_ACCEL].max)) {
    return false;
  }

  ThrottleLine *line = storage->calibration_data->line;

  int16_t max_main = line[THROTTLE_CHANNEL_MAIN].full_accel_reading;
  int16_t min_main = line[THROTTLE_CHANNEL_MAIN].full_brake_reading;

  int16_t max_secondary = line[THROTTLE_CHANNEL_SECONDARY].full_accel_reading;
  int16_t min_secondary = line[THROTTLE_CHANNEL_SECONDARY].full_brake_reading;

  int16_t tolerance = storage->calibration_data->tolerance;

  // This condition checks for edge cases where readings are valid but do not exist on the line.
  // For ex. if reading is below the line, it should be thought of as full_brake_reading.
  if (reading_main < min_main) {
    reading_main = min_main;
  } else if (reading_main > max_main) {
    reading_main = max_main;
  }

  // The document "Math behind Throttle Module" on Confluence explains the equation below.
  // Finds expected secondary reading that corresponds to the same pedal position as main reading.
  int16_t expected_reading_secondary =
      (max_secondary - min_secondary) * (reading_main - min_main) / (max_main - min_main) +
      min_secondary;

  // Checks if the secondary channel reading is within given bounds around the expected reading.
  return abs(expected_reading_secondary - reading_secondary) <= tolerance;
}

// The periodic callback which checks if readings are up to date and channels are in sync.
// If they are, the event corresponding to the pedals position is raised.
// If not, a pedal timout event is raised.
static void prv_raise_event_timer_callback(SoftTimerId timer_id, void *context) {
  ThrottleStorage *storage = context;
  int16_t reading_main = INT16_MIN;
  int16_t reading_secondary = INT16_MIN;

  bool fault = true;
  StatusCode primary_status = ads1015_read_raw(
      storage->pedal_ads1015_storage, storage->calibration_data->channel_main, &reading_main);
  StatusCode secondary_status =
      ads1015_read_raw(storage->pedal_ads1015_storage, storage->calibration_data->channel_secondary,
                       &reading_secondary);

  InputEvent pedal_events[NUM_THROTTLE_ZONES] = { INPUT_EVENT_PEDAL_BRAKE, INPUT_EVENT_PEDAL_COAST,
                                                  INPUT_EVENT_PEDAL_ACCEL };

  if (status_ok(primary_status) && status_ok(secondary_status) &&
      prv_channels_synced(reading_main, reading_secondary, storage)) {
    for (ThrottleZone zone = THROTTLE_ZONE_BRAKE; zone < NUM_THROTTLE_ZONES; zone++) {
      if (prv_reading_within_zone(reading_main, zone, storage)) {
        fault = false;
        storage->position.zone = zone;
        storage->position.numerator = prv_get_numerator_zone(reading_main, zone, storage);
        storage->reading_ok_flag = true;
        event_raise(pedal_events[zone], storage->position.numerator);
        break;
      }
    }
  }

  if (fault) {
    storage->reading_ok_flag = false;
    storage->position.zone = NUM_THROTTLE_ZONES;
    event_raise(INPUT_EVENT_PEDAL_FAULT, 0);
  }

  soft_timer_start_millis(THROTTLE_UPDATE_PERIOD_MS, prv_raise_event_timer_callback, context, NULL);
}

// Initializes the throttle by configuring the ADS1015 channels and
// setting the periodic safety check callback.
StatusCode throttle_init(ThrottleStorage *storage, ThrottleCalibrationData *calibration_data,
                         Ads1015Storage *pedal_ads1015_storage) {
  if (storage == NULL || calibration_data == NULL || pedal_ads1015_storage == NULL) {
    return status_code(STATUS_CODE_INVALID_ARGS);
  }
  memset(storage, 0, sizeof(*storage));

  storage->calibration_data = calibration_data;

  // The callback for updating flags is only set on the main channel.
  // Verifying later if the second channel is in sync with the main channel is sufficient.
  status_ok_or_return(ads1015_configure_channel(pedal_ads1015_storage,
                                                calibration_data->channel_main, true, NULL, NULL));
  status_ok_or_return(ads1015_configure_channel(
      pedal_ads1015_storage, calibration_data->channel_secondary, true, NULL, NULL));

  storage->pedal_ads1015_storage = pedal_ads1015_storage;

  return soft_timer_start_millis(THROTTLE_UPDATE_PERIOD_MS, prv_raise_event_timer_callback, storage,
                                 NULL);
}

// Gets the current position of the pedal (writes to position).
StatusCode throttle_get_position(ThrottleStorage *storage, ThrottlePosition *position) {
  if (storage == NULL || position == NULL) {
    return status_code(STATUS_CODE_INVALID_ARGS);
  }
  if (!storage->reading_ok_flag) {
    return status_code(STATUS_CODE_TIMEOUT);
  }

  bool disabled = critical_section_start();
  position->zone = storage->position.zone;
  position->numerator = storage->position.numerator;
  critical_section_end(disabled);

  return STATUS_CODE_OK;
}

ThrottleStorage *throttle_global(void) {
  return &s_throttle_storage;
}
