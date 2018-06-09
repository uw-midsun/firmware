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

// Uses a linear backoff. We aren't complex enough to warrant an exponential backoff.
#define EMERGENCY_FAULT_BACKOFF_MS 750

typedef struct EmergencyFaultStorage {
  SoftTimerID id;
  bool keep_trying;
} EmergencyFaultStorage;

// Simple utility to send a POWER_DISTRIBUTION_FAULT. Will auto retry until successfully ACK'd.
StatusCode emergency_fault_send(EmergencyFaultStorage *storage);

// Stops resending the message if transitioned to another state and still not ACK'd. It is highly
// unlikely this would happen as it requires one way comms between driver controls and power
// distribution.
void emergency_fault_clear(EmergencyFaultStorage *storage);

// Sends the message automatically when CHAOS_EVENT_SEQUENCE_EMERGENCY is detected in the event
// queue. Cancels on any other CHAOS_EVENT_SEQUENCE_* transition request.
void emergency_fault_process_event(EmergencyFaultStorage *storage, const Event *e);
