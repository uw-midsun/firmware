#include "charger_fsm.h"

#include <stddef.h>

#include "charger_controller.h"
#include "charger_events.h"
#include "fsm.h"
#include "notify.h"
#include "log.h"

static bool prv_safe_charging_guard(const Fsm *fsm, const Event *e, void *context) {
  (void)fsm;
  (void)e;
  (void)context;
  return charger_controller_is_safe();
}

FSM_DECLARE_STATE(state_disconnected);
FSM_DECLARE_STATE(state_connected);
FSM_DECLARE_STATE(state_charging);
FSM_DECLARE_STATE(state_error);

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

FSM_STATE_TRANSITION(state_error) {
  //transition out of error state should only transition to disconnected
  FSM_ADD_TRANSITION(CHARGER_EVENT_DISCONNECTED, state_disconnected);

}

static void prv_state_disconnected(Fsm *fsm, const Event *e, void *context) {
  LOG_DEBUG("ENTERED DISCONNECTED\n");
  (void)fsm;
  (void)e;
  (void)context;

  // No need for requests since the device is disconnected. Don't request charging.
  charger_controller_set_state(CHARGER_STATE_STOP);
  notify_cease();
}

static void prv_state_connected(Fsm *fsm, const Event *e, void *context) {
  LOG_DEBUG("ENTERED CONNECTED\n");

  (void)fsm;
  (void)e;
  (void)context;
  // Request charging but don't start the charger.
  charger_controller_set_state(CHARGER_STATE_STOP);
  notify_post();
}

static void prv_state_charging(Fsm *fsm, const Event *e, void *context) {
  LOG_DEBUG("ENTERED CHARGING\n");
  (void)fsm;
  (void)e;
  (void)context;

  // Start the charger and continue to request permissions.
  charger_controller_set_state(CHARGER_STATE_START);
}

static void prv_state_error(Fsm *fsm, const Event *e, void *context) {
  LOG_DEBUG("ENTERED ERROR\n");
  (void)fsm;
  (void)e;
  (void)context;

  charger_controller_set_state(CHARGER_STATE_STOP);
  notify_cease();
}

void charger_fsm_init(Fsm *fsm) {
  fsm_state_init(state_disconnected, prv_state_disconnected);
  fsm_state_init(state_connected, prv_state_connected);
  fsm_state_init(state_charging, prv_state_charging);
  fsm_state_init(state_error, prv_state_error);
  fsm_init(fsm, "ChargerFSM", &state_disconnected, NULL);
}
