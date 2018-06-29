#include "sequenced_relay.h"
#include <string.h>
#include "can_msg_defs.h"
#include "critical_section.h"
#include "exported_enums.h"
#include "soft_timer.h"

static void prv_delay_cb(SoftTimerID timer_id, void *context) {
  SequencedRelayStorage *storage = context;

  storage->delay_timer = SOFT_TIMER_INVALID_TIMER;
  // We only bother with the delay if we're closing the relays, so assume it's closing
  gpio_set_state(&storage->settings.right_relay, GPIO_STATE_HIGH);

  if (storage->settings.update_cb != NULL) {
    storage->settings.update_cb(EE_RELAY_STATE_CLOSE, storage->settings.context);
  }
}

static StatusCode prv_handle_relay_rx(SystemCanMessage msg_id, uint8_t state, void *context) {
  SequencedRelayStorage *storage = context;

  return sequenced_relay_set_state(storage, state);
}

StatusCode sequenced_relay_init(SequencedRelayStorage *storage,
                                const SequencedRelaySettings *settings) {
  if (storage == NULL || settings == NULL) {
    return status_code(STATUS_CODE_INVALID_ARGS);
  }
  memset(storage, 0, sizeof(*storage));
  storage->settings = *settings;
  storage->delay_timer = SOFT_TIMER_INVALID_TIMER;

  GPIOSettings gpio_settings = {
    .direction = GPIO_DIR_OUT,
    .state = GPIO_STATE_LOW,
  };
  gpio_init_pin(&storage->settings.left_relay, &gpio_settings);
  gpio_init_pin(&storage->settings.right_relay, &gpio_settings);

  return relay_rx_configure_handler(&storage->relay_rx, storage->settings.can_msg_id,
                                    NUM_EE_RELAY_STATES, prv_handle_relay_rx, storage);
}

StatusCode sequenced_relay_set_update_cb(SequencedRelayStorage *storage,
                                         SequencedRelayUpdateCb update_cb, void *context) {
  if (storage == NULL) {
    return status_code(STATUS_CODE_INVALID_ARGS);
  }

  bool disabled = critical_section_start();
  storage->settings.update_cb = update_cb;
  storage->settings.context = context;
  critical_section_end(disabled);

  return STATUS_CODE_OK;
}

StatusCode sequenced_relay_set_state(SequencedRelayStorage *storage, EERelayState state) {
  if (storage == NULL || state >= NUM_EE_RELAY_STATES) {
    return status_code(STATUS_CODE_INVALID_ARGS);
  }

  soft_timer_cancel(storage->delay_timer);

  gpio_set_state(&storage->settings.left_relay, (GPIOState)state);
  if (state == EE_RELAY_STATE_CLOSE) {
    soft_timer_start_millis(storage->settings.delay_ms, prv_delay_cb, storage,
                            &storage->delay_timer);
  } else {
    gpio_set_state(&storage->settings.right_relay, (GPIOState)state);
    if (storage->settings.update_cb != NULL) {
      storage->settings.update_cb(state, storage->settings.context);
    }
  }

  return STATUS_CODE_OK;
}
