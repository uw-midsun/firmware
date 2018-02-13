#include "status.h"
#include "event_queue.h"
#include "lights_events.h"
#include "fsm.h"
#include "process_periph.h"


static FSM s_signal_fsm_front;

// returns true if event's data is false (used for turning off a signal)
// static bool s_turn_off_guard(const FSM *fsm, const Event *e, void *context) {
//  return (bool) !e->data;
//}
//
// static bool s_turn_on_guard(const FSM *fsm, const Event *e, void *context) {
//  return (bool)e->data;
//}

// FSM_DECLARE_STATE(signal_left);
// FSM_DECLARE_STATE(signal_right);
// FSM_DECLARE_STATE(signal_hazard);
// FSM_DECLARE_STATE(signal_none);

// signal_left transition table
// FSM_STATE_TRANSITION(signal_left) {
//   FSM_ADD_GUARDED_TRANSITION(EVENT_SIGNAL_LEFT, s_turn_off_guard, signal_none);
//   FSM_ADD_GUARDED_TRANSITION(EVENT_SIGNAL_HAZARD, s_turn_on_guard, signal_hazard);
// }

// FSM_STATE_TRANSITION(signal_right) {
//   FSM_ADD_GUARDED_TRANSITION(EVENT_SIGNAL_RIGHT, s_turn_off_guard, signal_none);
//   FSM_ADD_GUARDED_TRANSITION(EVENT_SIGNAL_HAZARD, s_turn_on_guard, signal_hazard);
// }

StatusCode process_event(Event e) {
  InputEvent event = e.id;
  switch (event) {
    case EVENT_HORN:
    case EVENT_HEADLIGHTS:
    case EVENT_BRAKES:
    case EVENT_STROBE:
      return simple_peripheral(e);
      break;
    default:
      return STATUS_CODE_OK;
      break;
  }
}


