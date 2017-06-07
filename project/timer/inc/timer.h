#pragma once
#include <stdint.h>
#include "status.h"

#define SOFT_TIMER_MAX_TIMERS 15

typedef uint32_t SoftTimerID;
typedef void (*SoftTimerCb)(SoftTimerID timer_id, void *context);

StatusCode timer_init(void);

StatusCode timer_start(uint32_t time_us, SoftTimerCb callback, void *context,
                       SoftTimerID *timer_id);
