#pragma once
// Wrapper around CAN Hw for use of extended ids with Generic CAN.
// This performs the initialization of can_hw.
// Note this is primarily intended for testing. It isn't nearly as fault tolerant as CAN network.
//
// TODO(ELEC-355): Create extended support for th CAN Network layer.

#include "can_hw.h"
#include "event_queue.h"
#include "generic_can.h"
#include "status.h"

#define NUM_GENERIC_CAN_HW_RX_HANDLERS 5

typedef struct GenericCanHw {
  GenericCan base;
  GenericCanRxStorage rx_storage[NUM_GENERIC_CAN_HW_RX_HANDLERS];
  EventID fault_event;
} GenericCanHw;

StatusCode generic_can_hw_init(const CANHwSettings *settings, EventID fault_event,
                               GenericCanHw *out);
