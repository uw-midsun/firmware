#include "soft_timer.h"

#include <signal.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <time.h>

#include "critical_section.h"
#include "interrupt.h"
#include "status.h"
#include "x86_interrupt.h"

typedef struct Timer {
  uint32_t duration;
  void* context;
  bool inuse;
  SoftTimerCallback callback;
} Timer;

typedef struct POSIXTimer {
  timer_t timer_id;
  uint32_t duration_us;
  struct sigevent event;
  bool created;
  bool active;
} POSIXTimer;

static volatile Timer s_soft_timer_array[SOFT_TIMER_MAX_TIMERS];
static volatile SoftTimerID s_active_timer_id;
static POSIXTimer s_posix_timer;

static void prv_soft_timer_update(uint32_t curr_time) {
  // Default these values to max. If there is ever a newer timer it will replace them. In the event
  // there are no timers left the defaults will result in the module being reset to a state where no
  // timers exist.
  SoftTimerID min_time_id = SOFT_TIMER_MAX_TIMERS;
  uint32_t min_duration = UINT32_MAX;
  for (uint32_t i = 0; i < SOFT_TIMER_MAX_TIMERS; i++) {
    // If the timer is in use and wasn't just added then update its duration.
    if (s_soft_timer_array[i].inuse) {
      if (curr_time >= s_soft_timer_array[i].duration) {
        // For each inuse timer see if the timer expired. If it has run its callback.
        s_soft_timer_array[i].callback(i, s_soft_timer_array[i].context);
        s_soft_timer_array[i].inuse = false;
      } else {
        // Otherwise, update the duration left on the timer.
        s_soft_timer_array[i].duration -= curr_time;

        // Figure out the next timer.
        if (min_duration >= s_soft_timer_array[i].duration) {
          min_time_id = i;
          min_duration = s_soft_timer_array[i].duration;
        }
      }
    }
  }
  // Set the active timer id.
  s_active_timer_id = min_time_id;
}

static void prv_start_timer(uint32_t duration_us) {
  // Create an empty interrupt timerspec.
  struct itimerspec timerspec = { { 0, 0 }, { 0, 0 } };

  // Set the it_val to the value we want the interrupt to trigger on.
  timerspec.it_value.tv_sec = duration_us / 1000000;
  timerspec.it_value.tv_nsec = duration_us % 1000000 * 1000;

  // Set the time of the interrupt on the timer. 0 is the flag for using time relative to the
  // current clock time. NULL is since we don't care what the previous settings were.
  timer_settime(s_posix_timer.timer_id, 0, &timerspec, NULL);

  // Update the state of the timer.
  s_posix_timer.duration_us = duration_us;
  s_posix_timer.active = true;
}

static void prv_soft_timer_interrupt(void) {
  // Assume the next interrupt is the active one.
  bool critical = critical_section_start();
  prv_soft_timer_update(s_soft_timer_array[s_active_timer_id].duration);
  if (s_active_timer_id < SOFT_TIMER_MAX_TIMERS) {
    prv_start_timer(s_soft_timer_array[s_active_timer_id].duration);
  }
  critical_section_end(critical);
}

static void prv_soft_timer_handler(uint8_t interrupt_id) {
  // Run the interrupt since there is only one for this handler.
  prv_soft_timer_interrupt();
}

static uint32_t prv_soft_timer_get_time(void) {
  struct itimerspec curr_timespec;
  timer_gettime(s_posix_timer.timer_id, &curr_timespec);
  return s_posix_timer.duration_us -
         (curr_timespec.it_value.tv_sec * 1000000 + curr_timespec.it_value.tv_nsec / 1000);
}

void soft_timer_init(void) {
  // Delete the timer if it exists.
  if (s_posix_timer.created) {
    timer_delete(s_posix_timer.timer_id);
    s_posix_timer.created = false;
    s_posix_timer.active = false;
  }

  // Set all timers to make none appear active.
  s_active_timer_id = SOFT_TIMER_MAX_TIMERS;
  for (uint32_t i = 0; i < SOFT_TIMER_MAX_TIMERS; i++) {
    s_soft_timer_array[i].inuse = false;
  }

  // Register a handler and interrupt.
  uint8_t handler_id;
  x86_interrupt_register_handler(prv_soft_timer_handler, &handler_id);
  InterruptSettings it_settings = { .type = INTERRUPT_TYPE_INTERRUPT,
                                    .priority = INTERRUPT_PRIORITY_NORMAL };
  uint8_t interrupt_id;
  x86_interrupt_register_interrupt(handler_id, &it_settings, &interrupt_id);

  // Create the timer. It triggers an interrupt on the interrupt registered here.
  s_posix_timer.event.sigev_value.sival_int = interrupt_id;
  s_posix_timer.event.sigev_notify = SIGEV_SIGNAL;
  s_posix_timer.event.sigev_signo = SIGRTMIN + INTERRUPT_PRIORITY_NORMAL;
  timer_create(CLOCK_MONOTONIC, &s_posix_timer.event, &s_posix_timer.timer_id);
  s_posix_timer.created = true;
}

StatusCode soft_timer_start(uint32_t duration_us, SoftTimerCallback callback, void* context,
                            SoftTimerID* timer_id) {
  // Start a critical section to prevent this section from being broken.
  bool critical = critical_section_start();

  for (uint32_t i = 0; i < SOFT_TIMER_MAX_TIMERS; i++) {
    if (!s_soft_timer_array[i].inuse) {
      // Look for an empty timer.

      // Get the current time.
      uint32_t curr_time = 0;
      if (s_posix_timer.active) {
        curr_time = prv_soft_timer_get_time();
      }
      if (s_active_timer_id >= SOFT_TIMER_MAX_TIMERS) {
        // New timer will be the only timer. Start it.
        s_active_timer_id = i;
        prv_start_timer(duration_us);
      } else if (duration_us < s_soft_timer_array[s_active_timer_id].duration - curr_time) {
        // New timer will run be before the active timer. Update and start it. Use as close to now
        // as possible for the update.
        prv_soft_timer_update(curr_time);
        s_active_timer_id = i;
        prv_start_timer(duration_us);
      } else {
        // Update the timers to not interfere with the duration of this one.
        prv_soft_timer_update(curr_time);
        prv_start_timer(s_soft_timer_array[s_active_timer_id].duration - curr_time);
      }
      // Otherwise, the new timer is longest ignore it until later.

      // Actually populate the timer. Do this after the previous steps as this is a critical section
      // and this timer shouldn't be updated.
      s_soft_timer_array[i].duration = duration_us;
      s_soft_timer_array[i].callback = callback;
      s_soft_timer_array[i].context = context;
      s_soft_timer_array[i].inuse = true;
      *timer_id = i;
      critical_section_end(critical);
      return STATUS_CODE_OK;
    }
  }

  // Out of timers.
  critical_section_end(critical);
  return status_msg(STATUS_CODE_RESOURCE_EXHAUSTED, "Out of software timers.");
}

bool soft_timer_inuse(void) {
  if (s_active_timer_id != SOFT_TIMER_MAX_TIMERS) {
    return true;
  }
  return false;
}

bool soft_timer_cancel(SoftTimerID timer_id) {
  if (timer_id < SOFT_TIMER_MAX_TIMERS) {
    if (timer_id == s_active_timer_id) {
      // Start a critical section and update pretending the active timer doesn't exist so as to
      // replace it with the next timer.
      bool critical = critical_section_start();
      s_soft_timer_array[timer_id].inuse = false;

      prv_soft_timer_update(s_soft_timer_array[s_active_timer_id].duration -
                            prv_soft_timer_get_time());
      if (s_active_timer_id < SOFT_TIMER_MAX_TIMERS) {
        prv_start_timer(s_soft_timer_array[s_active_timer_id].duration);
      }
      critical_section_end(critical);
      return true;
    } else if (s_soft_timer_array[timer_id].inuse) {
      // Not in use pretend it doesn't exist
      s_soft_timer_array[timer_id].inuse = false;
      return true;
    }
  }
  return false;
}
