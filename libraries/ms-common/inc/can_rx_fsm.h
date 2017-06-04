#pragma once
#include "fsm.h"
#include "can.h"
// CAN TX/RX event handlers

// Register RX handlers?

// Idea: ISRs raise TX and RX events, push elements into queue
// RX FSM: On RX event, pop elements in RX queue
// If type = ACK, run ACK handler
// Else, run RX handler/callback? Or raise appropriate event - probably want callback to copy data
// If critical, send ACK?

// TX: request transmit - push message into queue, attempt to TX
// if critical, create ACK request and start timer

StatusCode can_rx_fsm_init(FSM *rx_fsm, CANConfig *can);
