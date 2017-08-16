#include "fsm.h"

// CAN TX FSM: Only attempts to transmit one message at a time - waits for each message
// to be successfully transmitted before sending the next one.
FSM_DECLARE_STATE(can_tx_fsm_idle);
FSM_DECLARE_STATE(can_tx_fsm_active);

// Event sources:
// We register TX ready with CAN HW - this is used to move into the idle state
// The output function of the idle state creates TX request events if there are any queued messages
// Note that in the active state, TX requests are just ignored - they will be recreated when a TX
// has completed.
// This doesn't take advantage of the multiple mailboxes, but should be a much more straightforward
// solution.
// Unforunately, the problem seems to be an issue with the transmission never actually succeeding
// on either side.
