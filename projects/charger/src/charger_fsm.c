#include "charger_fsm.h"

#include "charger_controller.h"
#include "fsm.h"
#include "permissions.h"

static FSM s_charger_fsm;

static bool prv_safe_charging_guard(const FSM *fsm, const Event *e, void *context) {
  ChargerStatus *status = context;
  // TODO(ELEC-355): Determine which statuses warrant blocking charging. Add a function to
  // |charger_controller| to check |status|.
}

FSM_DECLARE_STATE(state_disconnected);
FSM_DECLARE_STATE(state_connected);
FSM_DECLARE_STATE(state_charging);

FSM_STATE_TRANSITION(state_disconnected) {
  // TODO(ELEC-355): Fill in

  // Interrupt driven events on the charger pin.
  FSM_ADD_TRANSITION(0, state_connected);
}

FSM_STATE_TRANSITION(state_connected) {
  // TODO(ELEC-355): Fill in

  // Interrupt driven events on the charger pin.
  FSM_ADD_TRANSITION(0, state_disconnected);

  // Requires permissions and that the charger is in a safe state. This is guarded to prevent
  // re-entry into this state in the event permission is given but the charger is deemed unsafe.
  FSM_ADD_GUARDED_TRANSITION(0, prv_safe_charging_guard, state_charging);
}

FSM_STATE_TRANSITION(state_charging) {
  // TODO(ELEC-355): Fill in

  // Interrupt driven events on the charger pin.
  FSM_ADD_TRANSITION(0, state_disconnected);  // Controlled by charger_pin.

  // Controlled by permissions. Will be forcibly kicked to this state if the charger state is
  // identified as dangerous by the charger_controller module.
  FSM_ADD_TRANSITION(0, state_connected);
}

static void prv_state_disconnected(FSM *fsm, const Event *e, void *context) {
  permissions_cease_request();
  charger_set_state(CHARGER_STATE_STOP);
}

static void prv_state_connected(FSM *fsm, const Event *e, void *context) {
  permissions_request();
  charger_set_state(CHARGER_STATE_STOP);
}

static void prv_state_charging(FSM *fsm, const Event *e, void *context) {
  charger_set_state(CHARGER_STATE_START);
}

void charger_fsm_init(ChargerStatus *status) {
  fsm_state_init(state_disconnected, NULL);
  fsm_state_init(state_connected, NULL);
  fsm_state_init(state_charging, NULL);
  fsm_init(&s_charger_fsm, "ChargerFSM", &state_disconnected, (void *)status);
}

bool charger_fsm_process_event(const Event *e) {
  return fsm_process_event(&s_charger_fsm, e);
}
