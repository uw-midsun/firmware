// Updates to the drive output module are driven by the update requested events

#include "pedal_fsm.h"
#include "drive_output.h"
#include "event_arbiter.h"
#include "event_queue.h"
#include "log.h"
#include "pc_input_event.h"
#include "throttle.h"

// Pedal FSM state definitions
// We only really care about braking vs. not braking, but keep the 3 states just to be explicit
FSM_DECLARE_STATE(state_brake);
FSM_DECLARE_STATE(state_coast);
FSM_DECLARE_STATE(state_accel);

// Pedal FSM transition table definitions
FSM_STATE_TRANSITION(state_brake) {
  FSM_ADD_TRANSITION(INPUT_EVENT_DRIVE_UPDATE_REQUESTED, state_brake);

  FSM_ADD_TRANSITION(INPUT_EVENT_PEDAL_COAST, state_coast);
  FSM_ADD_TRANSITION(INPUT_EVENT_PEDAL_ACCEL, state_accel);
}

FSM_STATE_TRANSITION(state_coast) {
  FSM_ADD_TRANSITION(INPUT_EVENT_DRIVE_UPDATE_REQUESTED, state_coast);

  FSM_ADD_TRANSITION(INPUT_EVENT_PEDAL_BRAKE, state_brake);
  FSM_ADD_TRANSITION(INPUT_EVENT_PEDAL_ACCEL, state_accel);
}

FSM_STATE_TRANSITION(state_accel) {
  FSM_ADD_TRANSITION(INPUT_EVENT_DRIVE_UPDATE_REQUESTED, state_accel);

  FSM_ADD_TRANSITION(INPUT_EVENT_PEDAL_BRAKE, state_brake);
  FSM_ADD_TRANSITION(INPUT_EVENT_PEDAL_COAST, state_coast);
}

// Pedal FSM output functions
static void prv_update_drive_output(void) {
  ThrottlePosition position = { 0 };
  // TODO(ELEC-431): Could just remove UPDATE_REQUESTED transitions and use the actual events + data
  StatusCode status = throttle_get_position(throttle_global(), &position);
  if (!status_ok(status)) {
    return;
  }

  const int16_t zone_multiplier[NUM_THROTTLE_ZONES] = {
    [THROTTLE_ZONE_BRAKE] = -1,
    [THROTTLE_ZONE_COAST] = 0,
    [THROTTLE_ZONE_ACCEL] = 1,
  };

  int16_t throttle_position = zone_multiplier[position.zone] * position.numerator;
  drive_output_update(drive_output_global(), DRIVE_OUTPUT_SOURCE_THROTTLE, throttle_position);
}

static bool prv_brake_guard(const Event *e) {
  // Prevent entering cruise if braking
  return e->id != INPUT_EVENT_CONTROL_STALK_ANALOG_CC_RESUME;
}

// Pedal FSM output functions
static void prv_brake_output(Fsm *fsm, const Event *e, void *context) {
  EventArbiterGuard *guard = context;
  event_arbiter_set_guard_fn(guard, prv_brake_guard);

  // Mechanical brake can cause the throttle zone to be in a non-brake zone while in brake state
  // We'll still update the drive output with the throttle position for telemetry
  prv_update_drive_output();
}

static void prv_not_brake_output(Fsm *fsm, const Event *e, void *context) {
  EventArbiterGuard *guard = context;
  event_arbiter_set_guard_fn(guard, NULL);

  // Should just be coast/accel
  prv_update_drive_output();
}

StatusCode pedal_fsm_init(Fsm *fsm, EventArbiterStorage *storage) {
  fsm_state_init(state_brake, prv_brake_output);
  fsm_state_init(state_coast, prv_not_brake_output);
  fsm_state_init(state_accel, prv_not_brake_output);

  EventArbiterGuard *guard = event_arbiter_add_fsm(storage, fsm, prv_brake_guard);

  if (guard == NULL) {
    return status_code(STATUS_CODE_RESOURCE_EXHAUSTED);
  }

  fsm_init(fsm, "Pedal FSM", &state_brake, guard);

  return STATUS_CODE_OK;
}
