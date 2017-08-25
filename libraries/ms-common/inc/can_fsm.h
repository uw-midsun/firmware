#pragma once
// CAN TX/RX event handlers
// This is an internal module and should not be used.
#include "can.h"
#include "fsm.h"

// The idea is that the CAN RX ISR pushes received message into a queue and raises an RX event.
// In the FSM, when we encounter an RX event, we pop the messages in the RX queue
// and decide what to do based on the event type:
// * Data: Find registered RX handler and run the corresponding callback. If an ACK was requested,
//         we send an ACK as a response. The callback may choose to set the ACK's status.
// * ACK: Find the corresponding pending ACK request and process the ACK.

// TODO: Update with documentation on TX events
// basically the idea is that we raise TX events and process them in the main loop instead of
// just chaining it off of the TX ready interrupt to prevent transmits from starving the main loop.

StatusCode can_fsm_init(FSM *fsm, CANStorage *can_storage);
