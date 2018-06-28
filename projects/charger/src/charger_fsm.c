#include "charger_fsm.h"

#include <stddef.h>

#include "charger_controller.h"
#include "charger_events.h"
#include "fsm.h"
#include "log.h"
#include "notify.h"

static bool prv_safe_charging_guard(const FSM *fsm, const Event *e, void *context) {
  (void)fsm;
  (void)e;
  (void)context;
  return charger_controller_is_safe();
}

FSM_DECLARE_STATE(state_disconnected);
FSM_DECLARE_STATE(state_connected);
FSM_DECLARE_STATE(state_charging);

FSM_STATE_TRANSITION(state_disconnected) {
  // Interrupt driven events on the charger pin.
  FSM_ADD_TRANSITION(CHARGER_EVENT_CONNECTED, state_connected);
}

FSM_STATE_TRANSITION(state_connected) {
  // Interrupt driven events on the charger pin.
  FSM_ADD_TRANSITION(CHARGER_EVENT_DISCONNECTED, state_disconnected);

  // Requires permissions and that the charger is in a safe state. This is guarded to prevent
  // re-entry into this state in the event permission is given but the charger is deemed unsafe.
  FSM_ADD_GUARDED_TRANSITION(CHARGER_EVENT_START_CHARGING, prv_safe_charging_guard, state_charging);
}

FSM_STATE_TRANSITION(state_charging) {
  // Interrupt driven events on the charger pin.
  FSM_ADD_TRANSITION(CHARGER_EVENT_DISCONNECTED, state_disconnected);  // Controlled by charger_pin.

  // Controlled by permissions. Will be forcibly kicked to this state if the charger state is
  // identified as dangerous by the charger_controller module.
  FSM_ADD_TRANSITION(CHARGER_EVENT_STOP_CHARGING, state_connected);
}

static void prv_state_disconnected(FSM *fsm, const Event *e, void *context) {
  (void)fsm;
  (void)e;
  (void)context;
  LOG_DEBUG("Disconnected\n");
  // No need for requests since the device is disconnected. Don't request charging.
  charger_controller_set_state(CHARGER_STATE_STOP);
  notify_cease();
}

static void prv_state_connected(FSM *fsm, const Event *e, void *context) {
  (void)fsm;
  (void)e;
  (void)context;
  LOG_DEBUG("Connected\n");
  // Request charging but don't start the charger.
  charger_controller_set_state(CHARGER_STATE_STOP);
  notify_post();
}

static void prv_state_charging(FSM *fsm, const Event *e, void *context) {
  (void)fsm;
  (void)e;
  (void)context;
  LOG_DEBUG("Charging\n");
  // Start the charger and continue to request permissions.
  charger_controller_set_state(CHARGER_STATE_START);
}

void charger_fsm_init(FSM *fsm) {
  fsm_state_init(state_disconnected, prv_state_disconnected);
  fsm_state_init(state_connected, prv_state_connected);
  fsm_state_init(state_charging, prv_state_charging);
  fsm_init(fsm, "ChargerFSM", &state_disconnected, NULL);
}
