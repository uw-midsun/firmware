#include "soft_timer.h"

#include <stdbool.h>
#include <stdint.h>
#include <time.h>

#include "status.h"

typedef struct timer {
  uint32_t duration;
  uint64_t time_started_usecs;
  soft_timer_callback callback;
  bool inuse;
  void* context;
} timer;

static timer s_soft_timer_array[SOFT_TIMER_NUM];
static bool s_soft_timer_initialized = false;

void soft_timer_init(uint8_t clock_speed) {
  if (!s_soft_timer_initialized) {
    for (uint8_t i = 0; i < SOFT_TIMER_NUM; i++) {
      s_soft_timer_array[i].inuse = false;
    }
  }
}

StatusCode soft_timer_start(uint32_t duration_micros, soft_timer_callback callback,
                            uint8_t* timer_id, void* context) {
  struct timespec curr_time;
  clock_gettime(CLOCK_MONOTONIC, &curr_time);
  for (uint8_t i = 0; i < SOFT_TIMER_NUM; i++) {
    if (!s_soft_timer_array[i].inuse) {
      // Populate next available timer if possible.
      s_soft_timer_array[i].duration = duration_micros;
      s_soft_timer_array[i].time_started_usecs =
          curr_time.tv_sec * 1000000 + curr_time.tv_nsec / 1000;
      s_soft_timer_array[i].callback = callback;
      s_soft_timer_array[i].context = context;
      s_soft_timer_array[i].inuse = true;
      *timer_id = i;
      break;
    }
    if (i == SOFT_TIMER_NUM - 1) {
      return status_msg(STATUS_CODE_RESOURCE_EXHAUSTED, "Out of software timers.");
    }
  }
  return STATUS_CODE_OK;
}

void soft_timer_update(void) {
  struct timespec update_time;
  clock_gettime(CLOCK_MONOTONIC, &update_time);
  uint64_t update_time_usecs = update_time.tv_sec * 1000000 + update_time.tv_nsec / 1000;

  for (uint8_t i = 0; i < SOFT_TIMER_NUM; i++) {
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
