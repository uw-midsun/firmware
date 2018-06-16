#pragma once
// Sequencer FSM:
// A hierarchical FSM for Chaos. The concept is that the other FSMs in Chaos (other than CAN) act as
// a series of micro services which are controlled by the sequencer. The sequencer holds the overall
// state of the car and will retry "transitioning" to a state. If it encounters anything unexpected
// it will retry that transition until successful. In the event a transition fails it will enter
// the emergency state as some component is likely failing.

// Requires that event_queue is initialized.

#include "event_queue.h"

void sequencer_fsm_init(void);

// Publishes the next event in the sequence and transitions to a new sequence if needed. Errors if
// provided event was unexpected and will attempt to restart the current sequence.
//
// |previous_event| is the event that was just processed by theevent loop. It will be handled by
// this module if it is either a sequence transition. If it is an expected FSM event then it will be
// handled, if it is not an FSM event it will be skipped. If it is an unexpected FSM event the
// sequence will restart.
//
// NOTE: this implicitly means any events in the range
// (NUM_CHAOS_EVENTS_CAN, NUM_CHAOS_EVENTS_FSM) must originate from the sequencer!!! This is to
// ensure redundancy and allow retry attempts. If this fails 3 times consecutively then a serious
// fault has occurred with Chaos. We enter the emergency state as something terrible has happened.
// Realistically this is almost impossible and would only be triggered by relays repeatedly
// failing to transition.
//
// Example flow:
//
// ----------------------
// | Transition request |
// ----------------------
//           |       ______________
//           |       |            |
//           V       V            |
// ----------------------         |
// | event_process(&e); |         |
// ----------------------         |
//           |       ________     |
//           |       |      |     |
//           V       V      |     |
// ----------------------   |     |
// | Pass to other FSMs |   |     |
// ----------------------   |     |
//           |       |______|     |
//           |                    |
//           V                    |
// -------------------------------------
// | sequencer_publish_next_event(&e); |
// -------------------------------------
//           |
//           |
//           V
// -----------------------------
// | Finished state transition |
// -----------------------------
StatusCode sequencer_fsm_publish_next_event(const Event *previous_event);
