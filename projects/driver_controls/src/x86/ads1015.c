#include "ads1015.h"
#include <string.h>
#include "ads1015_def.h"
#include "gpio_it.h"
#include "soft_timer.h"

#define ADS1015_CHANNEL_UPDATE_PERIOD 30

// Checks if a channel is enabled (true) or disabled (false).
static bool channel_is_enabled(Ads1015Storage *storage, Ads1015Channel channel) {
  return ((storage->channel_bitset & (1 << channel)) != 0);
}

// Updates the channel_bitset when a channel is enabled/disabled.
static void prv_mark_channel_enabled(Ads1015Channel channel, bool enable, uint8_t *channel_bitset) {
  if (enable) {
    *channel_bitset |= (1 << channel);
  } else {
    *channel_bitset &= ~(1 << channel);
  }
}

// Sets the current channel of the storage.
static StatusCode prv_set_channel(Ads1015Storage *storage, Ads1015Channel channel) {
  if (channel >= NUM_ADS1015_CHANNELS) {
    return status_code(STATUS_CODE_INVALID_ARGS);
  }

  storage->current_channel = channel;
  return STATUS_CODE_OK;
}

// Periodically calls channels' callbacks imitating the interrupt behavior.
static void prv_channel_callback_caller(SoftTimerID id, void *context) {
  Ads1015Storage *storage = context;
  Ads1015Channel current_channel = storage->current_channel;
  uint8_t channel_bitset = storage->channel_bitset;

  if (channel_is_enabled(storage, current_channel)) {
    storage->channel_readings[current_channel] = 0;
    // Runs the users callback if not NULL.
    if (storage->channel_callback[current_channel] != NULL) {
      storage->channel_callback[current_channel](current_channel,
                                                 storage->callback_context[current_channel]);
    }
  }

  prv_mark_channel_enabled(current_channel, false, &storage->pending_channel_bitset);
  if (storage->pending_channel_bitset == ADS1015_BITSET_EMPTY) {
    // Reset the pending bitset once gone through a cycle of channel rotation.
    storage->pending_channel_bitset = channel_bitset;
  }
  current_channel = __builtin_ffs(storage->pending_channel_bitset) - 1;
  // Update so that the ADS1015 reads from the next channel.
  prv_set_channel(storage, current_channel);
  soft_timer_start(ADS1015_CHANNEL_UPDATE_PERIOD, prv_channel_callback_caller, storage, NULL);
}

StatusCode ads1015_init(Ads1015Storage *storage, I2CPort i2c_port, Ads1015Address i2c_addr,
                        GPIOAddress *ready_pin) {
  if ((storage == NULL) || (ready_pin == NULL)) {
    return status_code(STATUS_CODE_INVALID_ARGS);
  }
  memset(storage, 0, sizeof(*storage));
  for (Ads1015Channel channel = 0; channel < NUM_ADS1015_CHANNELS; channel++) {
    storage->channel_readings[channel] = ADS1015_DISABLED_CHANNEL_READING;
  }

  return STATUS_CODE_OK;
}

// Enable/disables a channel, and sets a callback for the channel.
StatusCode ads1015_configure_channel(Ads1015Storage *storage, Ads1015Channel channel, bool enable,
                                     Ads1015Callback callback, void *context) {
  if (storage == NULL || channel >= NUM_ADS1015_CHANNELS) {
    return status_code(STATUS_CODE_INVALID_ARGS);
  }
  uint8_t channel_bitset = storage->channel_bitset;
  prv_mark_channel_enabled(channel, enable, &storage->channel_bitset);
  storage->pending_channel_bitset = storage->channel_bitset;
  storage->channel_callback[channel] = callback;
  storage->callback_context[channel] = context;

  if (enable) {
    soft_timer_start(ADS1015_CHANNEL_UPDATE_PERIOD, prv_channel_callback_caller, storage, NULL);
  } else {
    storage->channel_readings[channel] = ADS1015_DISABLED_CHANNEL_READING;
  }
  return STATUS_CODE_OK;
}

// Reads raw results from the storage.
StatusCode ads1015_read_raw(Ads1015Storage *storage, Ads1015Channel channel, int16_t *reading) {
  if (channel >= NUM_ADS1015_CHANNELS || storage == NULL || reading == NULL ||
      !channel_is_enabled(storage, channel)) {
    return status_code(STATUS_CODE_INVALID_ARGS);
  }

  *reading = storage->channel_readings[channel];
  return STATUS_CODE_OK;
}

// Reads conversion value in mVolt.
StatusCode ads1015_read_converted(Ads1015Storage *storage, Ads1015Channel channel,
                                  int16_t *reading) {
  if (channel >= NUM_ADS1015_CHANNELS || storage == NULL || reading == NULL ||
      !channel_is_enabled(storage, channel)) {
    return status_code(STATUS_CODE_INVALID_ARGS);
  }

  int16_t raw_reading = ADS1015_READ_UNSUCCESSFUL;
  status_ok_or_return(ads1015_read_raw(storage, channel, &raw_reading));
  *reading = (raw_reading * ADS1015_CURRENT_FSR) / ADS1015_NUMBER_OF_CODES;
  return STATUS_CODE_OK;
}
