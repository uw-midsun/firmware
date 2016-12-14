#pragma once
// FSM Interface
//
// For every event that is processed (i.e. results in a transition), the new state's
// output function is called.
//
// The FSM keeps track of both current and last states for debug purposes. It would be trivial to
// additionally track the last processed event.
//
// Usage - Forward declare all states with FSM_DECLARE_STATE:
//
// FSM_DECLARE_STATE(state_a);
// FSM_DECLARE_STATE(state_b);
//
// Then define the transition table for each state:
//
// FSM_STATE_TRANSITION(state_a) {
//   FSM_ADD_TRANSITION(0, state_b);
// }
//
// FSM_STATE_TRANSITION(state_b) {
//   FSM_ADD_TRANSITION(0, state_a);
//   FSM_ADD_TRANSITION(1, state_b);
// }
//
// Use fsm_state_init to set a state's output function (called whenever transitioned to).
#include <stdbool.h>
#include <stdint.h>
#include "event_queue.h"
#include "fsm_impl.h"

// Forward-declares the state for use.
#define FSM_DECLARE_STATE(state) _FSM_DECLARE_STATE(state)
// Defines the state transition table.
#define FSM_STATE_TRANSITION(state) _FSM_STATE_TRANSITION(state)
// Adds an entry to the state transition table.
#define FSM_ADD_TRANSITION(event_id, state) _FSM_ADD_TRANSITION(event_id, state)

// Initializes an FSM state with an output function.
#define fsm_state_init(state, output_func) _fsm_state_init(state, output_func)

struct FSM;
typedef void (*StateOutput)(struct FSM *fsm, const Event *e);
typedef void (*StateTransition)(struct FSM *fsm, const Event *e, bool *transitioned);

typedef struct State {
  const char *name;
  StateOutput output;
  StateTransition table;
} State;

typedef struct FSM {
  const char *name;
  State *last_state;
  State *current_state;
} FSM;

// Initializes the FSM.
void fsm_init(FSM *fsm, const char *name, State *default_state);

// Returns whether a transition occurred in the FSM.
bool fsm_process_event(FSM *fsm, const Event *e);
