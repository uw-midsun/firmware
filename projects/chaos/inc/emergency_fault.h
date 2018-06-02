#pragma once
// Module that wraps CAN_TRANSMIT_POWER_DISTRIBUTION_FAULT.
//
// Requires CAN, event_queue, and soft_timers to be initialized.
//
// Sends an emergency fault to synchronize with driver controls. Will keep retrying to send until
// successfully ACK'd.

#include <stdbool.h>

#include "event_queue.h"
#include "soft_timer.h"
#include "status.h"

typedef struct EmergencyFaultStorage {
  SoftTimerID id;
  bool keep_trying;
} EmergencyFaultStorage;

// Simple a utility to send a POWER_DISTRIBUTION_FAULT and auto retry until successfully ACK'd.
StatusCode emergency_fault_send(EmergencyFaultStorage *storage);

// Stops resending the message if transitioned to IDLE and still not ACK'd. Highly unlikely this
// would happen as it require one way comms between driver controls and power distribution.
void emergency_fault_clear(EmergencyFaultStorage *storage);

// Sends the event automatically when detected in the main event queue.
void emergency_fault_process_event(EmergencyFaultStorage *storage, const Event *e);
