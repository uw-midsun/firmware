#pragma once
// A module to schedule a delay without blocking the main loop.
//
// Requires event_queue and soft_timers to be initialized.
//
// There is only one global delay instance possible.
// TODO(ELEC-459): Allow multiple delay services to exist.

#include "event_queue.h"

// On CHAOS_EVENT_DELAY_MS a delay is scheduled for the value in the data field interpreted as
// milliseconds. After expiry CHAOS_EVENT_DELAY_DONE is raised.
void delay_service_process_event(const Event *e);

void delay_service_cancel(void);
