#pragma once
#include <stdint.h>
#include <stdbool.h>
#include "status.h"

#define SOFT_TIMER_MAX_TIMERS 15

typedef uint32_t SoftTimerID;
typedef void (*SoftTimerCb)(SoftTimerID timer_id, void *context);

StatusCode timer_init(void);

StatusCode timer_start(uint32_t time_us, SoftTimerCb callback, void *context,
                       SoftTimerID *timer_id);

bool timer_cancel(SoftTimerID timer_id);

bool timer_inuse(void);

uint32_t timer_remaining_time(SoftTimerID timer_id);
