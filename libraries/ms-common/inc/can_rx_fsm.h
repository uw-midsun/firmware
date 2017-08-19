#pragma once
// CAN RX event handlers
#include "can.h"
#include "fsm.h"

// The idea is that the CAN RX ISR pushes received message into a queue and raises an RX event.
// In the RX FSM, when we encounter an RX event, we pop the messages in the RX queue
// and decide what to do based on the event type:
// * Data: Find registered RX handler and run the corresponding callback. If an ACK was requested,
//         we send an ACK as a response. The callback may choose to set the ACK's status.
// * ACK: Find the corresponding pending ACK request and process the ACK.

StatusCode can_rx_fsm_init(FSM *rx_fsm, CANConfig *can);
