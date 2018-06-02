#include "motor_relay.h"
#include <string.h>
#include "can_msg_defs.h"
#include "exported_enums.h"
#include "soft_timer.h"

static void prv_delay_cb(SoftTimerID timer_id, void *context) {
  MotorRelayStorage *storage = context;

  // We only bother with the delay if we're closing the relays, so assume it's closing
  gpio_set_state(&storage->settings.right_relay, GPIO_STATE_HIGH);
}

static StatusCode prv_handle_relay_rx(SystemCanMessage msg_id, uint8_t state, void *context) {
  MotorRelayStorage *storage = context;

  soft_timer_cancel(storage->delay_timer);

  gpio_set_state(&storage->settings.left_relay, state);
  if (state == EE_RELAY_STATE_CLOSE) {
    soft_timer_start_millis(storage->settings.delay_ms, prv_delay_cb, storage,
                            &storage->delay_timer);
  } else {
    gpio_set_state(&storage->settings.right_relay, state);
  }

  return STATUS_CODE_OK;
}

StatusCode motor_relay_init(MotorRelayStorage *storage, const MotorRelaySettings *settings) {
  memset(storage, 0, sizeof(*storage));
  storage->settings = *settings;
  storage->delay_timer = SOFT_TIMER_INVALID_TIMER;

  GPIOSettings gpio_settings = {
    .direction = GPIO_DIR_OUT,
    .state = GPIO_STATE_LOW,
  };
  gpio_init_pin(&storage->settings.left_relay, &gpio_settings);
  gpio_init_pin(&storage->settings.right_relay, &gpio_settings);

  return relay_rx_configure_handler(&storage->relay_rx, SYSTEM_CAN_MESSAGE_MOTOR_RELAY,
                                    NUM_EE_RELAY_STATES, prv_handle_relay_rx, storage);
}
