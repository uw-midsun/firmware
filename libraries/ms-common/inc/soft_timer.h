#pragma once

#include <stdbool.h>
#include <stdint.h>

#include "status.h"

#define SOFT_TIMER_MAX_TIMERS 10

typedef uint8_t SoftTimerID;

typedef void (*SoftTimerCallback)(SoftTimerID timer_id, void* context);

// Initializes a set of software timers. Clock speed should be in MHz. Subsequent calls will do
// nothing. The clock speed is that of the external PLL crystal.
void soft_timer_init(void);

// Adds a software timer. The provided duration is the number of microseconds before running and the
// callback is the process to run once the time has expired. The timer_id is set to the id of the
// timer that will run the callback.
StatusCode soft_timer_start(uint32_t duration_us, SoftTimerCallback callback, void* context,
                            SoftTimerID* timer_id);

// Updates the software timer running any callbacks that the time had expired for. Call this in your
// main loop if using software timers.
void soft_timer_update(void);

// Checks if software timers are running, returns true if any soft timers are in use.
bool soft_timer_inuse(void);
