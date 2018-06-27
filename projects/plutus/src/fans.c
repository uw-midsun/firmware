#include "fans.h"
#include "fsm.h"
#include "plutus_event.h"
#include "can.h"
#include <string.h>

static FansSettings s_fans;
// Latch fault
static bool s_faulted;

static void prv_update_fans(bool on) {
  for (size_t i = 0; i < PLUTUS_CFG_NUM_FANS; i++) {
    gpio_set_state(&s_fans.fans[i], (on ? GPIO_STATE_HIGH : GPIO_STATE_LOW));
  }
}

static void prv_update_relay(EERelayState state, void *context) {
  if (state == EE_RELAY_STATE_CLOSE) {
    // Assume that if we closed the relays we probably cleared the fault
    s_faulted = false;
    prv_update_fans(true);
  } else if (!s_faulted) {
    // If we didn't fault, turn off the fans when the relays open.
    prv_update_fans(false);
  }
}

StatusCode fans_init(const FansSettings *settings) {
  if (settings == NULL) {
    return status_code(STATUS_CODE_INVALID_ARGS);
  }

  s_fans = *settings;

  const GPIOSettings gpio_settings = {
    .direction = GPIO_DIR_OUT,
    .state = GPIO_STATE_LOW,
  };
  for (size_t i = 0; i < PLUTUS_CFG_NUM_FANS; i++) {
    status_ok_or_return(gpio_init_pin(&s_fans.fans[i], &gpio_settings));
  }

  // TODO: Add fan request message
  // can_register_rx_handler()

  return sequenced_relay_set_update_cb(settings->relays, prv_update_relay, NULL);
}

bool fans_process_event(const Event *e) {
  if (e->id == PLUTUS_EVENT_BPS_FAULT) {
    prv_update_fans(true);
    s_faulted = true;
  }

  return false;
}
