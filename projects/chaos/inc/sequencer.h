#pragma once
// Sequencer:
// A hierarchical FSM for Chaos. The concept is that the other FSMs in Chaos (other than CAN) act as
// a series of micro services which are controlled by the sequencer. The sequencer holds the overall
// state of the car and will retry "transitioning" to a state. If it encounters anything unexpected
// it will retry that transition until successful. In the event a transition fails it will enter
// the emergency state as some component is likely failing.

// Requires that event_queue is initialized.

#include "event_queue.h"

void sequencer_init(void);

// Publishes the next event in the sequence and transitions to a new sequence if needed. Errors if
// provided event was unexpected and will attempt to restart the current sequence.
//
// |previous_event| is the event that was just processed by the main event loop. It will be handled
// by this module if it is either a sequence transition. If it is an expected FSM event then it will
// be handled, if it is not an FSM event it will be skipped. If it is an unexpected FSM event the
// sequence will restart.
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
StatusCode sequencer_publish_next_event(const Event *previous_event);
