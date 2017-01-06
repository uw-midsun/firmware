#pragma once

#include <stdint.h>

#include "status.h"

#define SOFT_TIMER_NUM 10

typedef void (*soft_timer_callback)(uint8_t timer_id, void* context);

// Initializes a set of software timers. Clock speed should be in MHz. Subsequent calls will do
// nothing. The clock speed is that of the external PLL crystal.
void soft_timer_init(uint8_t clock_speed);

// Adds a software timer. The provided duration is the number of microseconds before running and the
// callback is the process to run once the time has expired. The timer_id is set to the id of the
// timer that will run the callback.
StatusCode soft_timer_start(uint32_t duration_micros, soft_timer_callback callback,
                            uint8_t* timer_id, void* context);

// Updates the software timer running any callbacks that the time had expired for. Call this in your
// main loop if using software timers.
void soft_timer_update(void);
