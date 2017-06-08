#pragma once
#include <stdint.h>
#include <stdbool.h>
#include "status.h"

#define SOFT_TIMER_MAX_TIMERS 15
#define SOFT_TIMER_INVALID_TIMER (SOFT_TIMER_MAX_TIMERS)

typedef uint32_t SoftTimerID;
typedef void (*SoftTimerCb)(SoftTimerID timer_id, void *context);

StatusCode timer_init(void);

StatusCode timer_start(uint32_t duration_us, SoftTimerCb callback, void *context,
                       SoftTimerID *timer_id);

#define timer_start_ms(duration_ms, callback, context, timer_id) \
  timer_start((duration_ms) * 1000, (callback), (context), (timer_id))

#define timer_start_secs(duration_secs, callback, context, timer_id) \
  timer_start((duration_secs) * 1000000, (callback), (context), (timer_id))

bool timer_cancel(SoftTimerID timer_id);

bool timer_inuse(void);

uint32_t timer_remaining_time(SoftTimerID timer_id);
