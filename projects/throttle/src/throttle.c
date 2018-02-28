#include "throttle.h"


#define THROTTLE_UPDATE_TIMEOUT_MS 50 // arbitrary for now

static void prv_flag_update_callback(Ads1015Channel channel, void *context) {
  if (context == NULL) return;
  ThrottleStorage *throttle_storage = context;
  throttle_storage->reading_updated_flag = true;
  throttle_storage->reading_ok_flag = true;
}

StatusCode throttle_init(ThrottleStorage *throttle_storage, Ads1015Storage *pedal_ads1015_storage,
                         Ads1015Channel channel_0, Ads1015Channel channel_1) {
  if (throttle_storage == NULL || pedal_ads1015_storage == NULL ||
      channel_0 >= NUM_ADS1015_CHANNELS || channel_1 >= NUM_ADS1015_CHANNELS) {
    return status_code(STATUS_CODE_INVALID_ARGS);
  }
  memset(throttle_storage, 0, sizeof(*throttle_storage));

  status_ok_or_return(ads1015_configure_channel(pedal_ads1015_storage, channel_0, true,
                                                prv_flag_update_callback, throttle_storage));
  status_ok_or_return(ads1015_configure_channel(pedal_ads1015_storage, channel_1, true,
                                                prv_flag_update_callback, throttle_storage));

  throttle_storage->pedal_ads1015_storage = pedal_ads1015_storage;
  throttle_storage->channels[0] = channel_0;
  throttle_storage->channels[1] = channel_1;
  return soft_timer_start_millis(THROTTLE_UPDATE_TIMEOUT_MS,
                                 prv_throttle_raise_event_timer_callback, throttle_storage,
                                 &throttle->raise_event_timer_id);
}

StatusCode throttle_get_position(ThrottleStorage *throttle_storage, position, percentage) {
  if (throttle_storage == NULL || position == NULL || percentage = NULL) {
    return status_code(STATUS_CODE_INVALID_ARGS);
  }
  if (throttle_storage->reading_ok_flag) {
    int16_t reading_0;
    int16_t reading_1;
    status_ok_or_return(ads1015_read_raw(throttle_storage->pedal_ads1015_storage,
                                         throttle_storage->channels[0], &reading_0));
    status_ok_or_return(ads1015_read_raw(throttle_storage->pedal_ads1015_storage,
                                         throttle_storage->channels[1], &reading_1));
    // TODO: scale the readings relative to upper and lower bounds and get the state and percentage.
    // TODO: verify channels are in synch.
    return STATUS_CODE_OK;
  }
  return status_code(STATUS_CODE_TIMEOUT);
}

static void prv_raise_event_timer_callback(SoftTimerID timer_id, void *context) {
  if (context == NULL) {
    return status_code(STATUS_CODE_INVALID_ARGS);
  }
  ThrottleStorage *throttle_storage = context;
  if (throttle_storage->reading_updated_flag) {
    // todo raise good event;
    // update position
  } else {
    // todo raise bad event;
    throttle_storage->reading_ok_flag =
        throttle_storage->reading_updated_flag;  // Only necessary here since if true, both get
                                                 // updated in prv_flag_update_callback
  }
  throttle_storage->reading_updated_flag = false;
  soft_timer_start_millis(THROTTLE_UPDATE_PERIOD_MS, prv_raise_event_timer_callback, context,
                          &throttle->raise_event_timer_id);
}
