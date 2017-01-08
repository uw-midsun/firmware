#include "soft_timer.h"

#include <stdbool.h>
#include <stdint.h>
#include <time.h>

#include "status.h"

typedef struct timer {
  uint64_t time_started_usecs;
  uint32_t duration;
  void* context;
  bool inuse;
  SoftTimerCallback callback;
} timer;

static timer s_soft_timer_array[SOFT_TIMER_MAX_TIMERS];

void soft_timer_init(void) {
  for (uint32_t i = 0; i < SOFT_TIMER_MAX_TIMERS; i++) {
    s_soft_timer_array[i].inuse = false;
  }
}

StatusCode soft_timer_start(uint32_t duration_us, SoftTimerCallback callback, void* context,
                            SoftTimerID* timer_id) {
  struct timespec curr_time;
  clock_gettime(CLOCK_MONOTONIC, &curr_time);
  for (uint32_t i = 0; i < SOFT_TIMER_MAX_TIMERS; i++) {
    if (!s_soft_timer_array[i].inuse) {
      // Populate next available timer if possible.
      s_soft_timer_array[i].duration = duration_us;
      s_soft_timer_array[i].time_started_usecs =
          curr_time.tv_sec * 1000000 + curr_time.tv_nsec / 1000;
      s_soft_timer_array[i].callback = callback;
      s_soft_timer_array[i].context = context;
      s_soft_timer_array[i].inuse = true;
      *timer_id = i;
      return STATUS_CODE_OK;
    }
  }

  return status_msg(STATUS_CODE_RESOURCE_EXHAUSTED, "Out of software timers.");
}

void soft_timer_update(void) {
  struct timespec update_time;
  clock_gettime(CLOCK_MONOTONIC, &update_time);
  uint64_t update_time_usecs = update_time.tv_sec * 1000000 + update_time.tv_nsec / 1000;

  for (uint32_t i = 0; i < SOFT_TIMER_MAX_TIMERS; i++) {
    if (s_soft_timer_array[i].inuse) {
      // For each inuse timer see if the timer expired. If it has run its callback. Otherwise update
      // if it has rolled over.
      if ((uint64_t)update_time_usecs - s_soft_timer_array[i].time_started_usecs >
          s_soft_timer_array[i].duration) {
        s_soft_timer_array[i].callback(i, s_soft_timer_array[i].context);
        s_soft_timer_array[i].inuse = false;
      }
    }
  }
}

bool soft_timer_inuse(void) {
  for (uint32_t i = 0; i < SOFT_TIMER_MAX_TIMERS; i++) {
    if (s_soft_timer_array[i].inuse) {
      return true;
      ;
    }
  }
  return false;
}
