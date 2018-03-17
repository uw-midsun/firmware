#pragma once
// Wrapper around CAN Hw for use of extended ids with Generic CAN.
// This performs the initialization of can_hw.
// Note this is primarily intended for testing. It isn't nearly as fault tolerant as CAN network.
//
// TODO(ELEC-355): Create extended support for the CAN Network layer.

#include "can_hw.h"
#include "event_queue.h"
#include "generic_can.h"
#include "status.h"

typedef struct GenericCanHw {
  GenericCan base;
  EventID fault_event;
} GenericCanHw;

StatusCode generic_can_hw_init(GenericCanHw *can_hw, const CANHwSettings *settings,
                               EventID fault_event);
