#pragma once
// Module to notify BPS to turn off the fans when the car is in idle
//
// Requires CAN to be initialized.

#include <stdbool.h>

#include "event_queue.h"

// On CHAOS_EVENT_SEQUENCE_IDLE_DONE sends a fan off message.
bool fan_control_proccess_event(const Event *e);
