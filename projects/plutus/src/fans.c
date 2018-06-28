#include "fans.h"
#include <string.h>
#include "can.h"
#include "can_unpack.h"
#include "exported_enums.h"
#include "fsm.h"
#include "plutus_event.h"

typedef struct FansStorage {
  FansSettings settings;
  SoftTimerID fault_timeout;
  EERelayState relay_state;
  bool faulted;
} FansStorage;

static FansStorage s_fan_storage;

static void prv_update_fans(bool on) {
  for (size_t i = 0; i < PLUTUS_CFG_NUM_FANS; i++) {
    gpio_set_state(&s_fan_storage->settings.fans[i], (on ? GPIO_STATE_HIGH : GPIO_STATE_LOW));
  }
}

static void prv_update_relay(EERelayState state, void *context) {
  if (state == EE_RELAY_STATE_CLOSE) {
    // Assume that if we closed the relays we probably cleared the fault
    s_fan_storage->faulted = false;
    prv_update_fans(true);
  } else if (!s_fan_storage->faulted) {
    // If we didn't fault, turn off the fans when the relays open.
    prv_update_fans(false);
  }

  s_fan_storage->relay_state = state;
}

static StatusCode prv_handle_fan_ctrl(const CANMessage *msg, void *context,
                                      CANAckStatus *ack_reply) {
  uint8_t fan_state = 0;
  CAN_UNPACK_FAN_CONTROL(msg, &fan_state);

  // The fan control state only takes effect if the relays are open. They must be on while the pack
  // is connected. If we're still faulting, the BPS heartbeat will raise a BPS fault event to turn
  // the fans back on.
  if (s_fan_storage->relay_state == EE_RELAY_STATE_OPEN) {
    bool fans_on = fan_state == EE_FAN_CONTROL_STATE_ENABLE;
    prv_update_fans(fans_on);

    // Assume explicit control means we cleared the fault - if we were wrong, the next BPS fault
    // will re-latch the fault.
    s_fan_storage->faulted = false;
  }

  return STATUS_CODE_OK;
}

StatusCode fans_init(const FansSettings *settings) {
  if (settings == NULL) {
    return status_code(STATUS_CODE_INVALID_ARGS);
  }

  memset(&s_fan_storage, 0, sizeof(s_fan_storage));
  s_fan_storage.settings = *settings;

  const GPIOSettings gpio_settings = {
    .direction = GPIO_DIR_OUT,
    .state = GPIO_STATE_LOW,
  };
  for (size_t i = 0; i < PLUTUS_CFG_NUM_FANS; i++) {
    status_ok_or_return(gpio_init_pin(&s_fan_storage->settings.fans[i], &gpio_settings));
  }

  status_ok_return(
      can_register_rx_handler(SYSTEM_CAN_MESSAGE_FAN_CONTROL, prv_handle_fan_ctrl, NULL));
  return sequenced_relay_set_update_cb(s_fan_storage->settings.relays, prv_update_relay, NULL);
}

bool fans_process_event(const Event *e) {
  if (e->id == PLUTUS_EVENT_BPS_FAULT) {
    prv_update_fans(true);
    s_fan_storage->faulted = true;
  }

  return false;
}
