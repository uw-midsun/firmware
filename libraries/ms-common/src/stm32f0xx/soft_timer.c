#include "soft_timer.h"

#include <stdbool.h>
#include <stdint.h>

#include "critical_section.h"
#include "interrupt.h"
#include "status.h"
#include "stm32f0xx_interrupt.h"
#include "stm32f0xx_rcc.h"
#include "stm32f0xx_tim.h"

typedef struct Timer {
  uint32_t duration;
  void *context;
  bool inuse;
  SoftTimerCallback callback;
} Timer;

static volatile Timer s_soft_timer_array[SOFT_TIMER_MAX_TIMERS];
static volatile SoftTimerID s_active_timer_id;
static volatile uint32_t s_last_update_count;
static volatile uint32_t s_active_timer_duration;

static void prv_start_timer(uint32_t duration_us) {
  // Clear any pending interrupts, this will always occur in an interrupt or new timer getting
  // started in a critical section so we need to prevent an accidental trigger if the time expires
  // in the middle of the critical section and we started a new timer which falls before it.
  TIM_ClearITPendingBit(TIM2, TIM_IT_CC1);

  TIM_Cmd(TIM2, DISABLE);
  uint32_t curr_time = TIM_GetCounter(TIM2);
  if (curr_time + duration_us > UINT32_MAX) {
    TIM_SetCompare1(TIM2, curr_time + duration_us - UINT32_MAX);
  } else {
    TIM_SetCompare1(TIM2, curr_time + duration_us);
  }
  s_active_timer_duration = duration_us;
  TIM_Cmd(TIM2, ENABLE);
}

static uint32_t prv_soft_timer_update_period(void) {
  uint32_t counter = TIM_GetCounter(TIM2);
  if (counter < s_last_update_count) {
    return counter - s_last_update_count;
  } else {
    return counter + UINT32_MAX - s_last_update_count;
  }
}

static void prv_soft_timer_update(uint32_t time_since_update) {
  // Default these values to max. If there is ever a newer timer it will replace them. In the event
  // there are no timers left the defaults will result in the module being reset to a state where no
  // timers exist.
  s_last_update_count = TIM_GetCounter(TIM2);
  SoftTimerID min_time_id = SOFT_TIMER_MAX_TIMERS;
  uint32_t min_duration = UINT32_MAX;

  for (uint32_t i = 0; i < SOFT_TIMER_MAX_TIMERS; i++) {
    // If the timer is in use and wasn't just added then update its duration.
    if (s_soft_timer_array[i].inuse) {
      if (time_since_update >= s_soft_timer_array[i].duration) {
        // For each inuse timer see if the timer expired. If it has run its callback.
        s_soft_timer_array[i].callback(i, s_soft_timer_array[i].context);
        s_soft_timer_array[i].inuse = false;
      } else {
        // Otherwise, update the duration left on the timer.
        s_soft_timer_array[i].duration -= time_since_update;
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

void soft_timer_init(void) {
  // Start the PeiphClock
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);
  // Stop the timer if it was running and ensure only overflows trigger updates.
  // Also remove the ARR preload.
  TIM_Cmd(TIM2, DISABLE);
  // TIM_InternalClockConfig(TIM2);

  // Get the clock speed of the clocks to determine the SYSCLK speed which is what TIM2 uses.
  RCC_ClocksTypeDef clock_speeds;
  RCC_GetClocksFreq(&clock_speeds);

  // Configure each clock tick to be 1 microsecond from 0 to duration.
  TIM_TimeBaseInitTypeDef init_struct = { .TIM_Prescaler = clock_speeds.SYSCLK_Frequency / 1000000,
                                          .TIM_CounterMode = TIM_CounterMode_Up,
                                          .TIM_Period = UINT32_MAX,
                                          .TIM_ClockDivision = TIM_CKD_DIV1 };

  TIM_TimeBaseInit(TIM2, &init_struct);

  // Reset the counter to 0.
  TIM_SetCounter(TIM2, 0);

  // Disable all the timers and forget the last running timer.
  s_last_update_count = 0;
  s_active_timer_id = SOFT_TIMER_MAX_TIMERS;
  for (uint32_t i = 0; i < SOFT_TIMER_MAX_TIMERS; i++) {
    s_soft_timer_array[i].inuse = false;
  }

  // Enable the interrupts. TIM2 uses IRQ channel 15.
  stm32f0xx_interrupt_nvic_enable(15, INTERRUPT_PRIORITY_NORMAL);
  TIM_ITConfig(TIM2, TIM_IT_CC1, ENABLE);

  // Start the timer.
  TIM_Cmd(TIM2, ENABLE);
}

StatusCode soft_timer_start(uint32_t duration_us, SoftTimerCallback callback, void *context,
                            SoftTimerID *timer_id) {
  // Enable a critical section.
  bool critical = critical_section_start();
  for (uint32_t i = 0; i < SOFT_TIMER_MAX_TIMERS; i++) {
    if (!s_soft_timer_array[i].inuse) {
      // Look for an empty timer.

      // Get the time this was called.
      uint32_t update_elapsed_time = prv_soft_timer_update_period();
      if (s_active_timer_id >= SOFT_TIMER_MAX_TIMERS) {
        // New timer will be the only timer. Start it.
        s_active_timer_id = i;
        prv_start_timer(duration_us);
      } else if (duration_us <
                 s_soft_timer_array[s_active_timer_id].duration - update_elapsed_time) {
        // New timer will run be before the active timer. Update and start it. Use as close to now
        // as possible for the update.
        prv_soft_timer_update(prv_soft_timer_update_period());
        s_active_timer_id = i;
        prv_start_timer(duration_us);
      } else {
        // Update the current time so as to not impact this timer's duration.
        prv_soft_timer_update(update_elapsed_time);
        prv_start_timer(s_soft_timer_array[s_active_timer_id].duration -
                        prv_soft_timer_update_period());
      }
      // Otherwise, the new timer is longest ignore it until later.

      // Actually populate the timer. Do this after the previous steps as this is a critical section
      // and this timer shouldn't be updated in prv_soft_timer_update.
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

// TIM2 Interrupt handler as defined in stm32f0xx.h
void TIM2_IRQHandler(void) {
  if (TIM_GetITStatus(TIM2, TIM_IT_CC1) != RESET) {
    // Update the timer assuming the active timer expired.
    prv_soft_timer_update(s_active_timer_duration);

    // Start the next active timer if it exists as determined by the update.
    if (s_active_timer_id < SOFT_TIMER_MAX_TIMERS) {
      prv_start_timer(s_soft_timer_array[s_active_timer_id].duration);
      return;
    }

    // Clear the pending bit to stop a repeat trigger if there are no new timers.
    TIM_ClearITPendingBit(TIM2, TIM_IT_CC1);
  }
}

bool soft_timer_cancel(SoftTimerID timer_id) {
  if (timer_id < SOFT_TIMER_MAX_TIMERS) {
    if (timer_id == s_active_timer_id) {
      // Start a critical section and update pretending the active timer doesn't exist so as to
      // replace it with the next timer.
      bool critical = critical_section_start();
      s_soft_timer_array[timer_id].inuse = false;

      prv_soft_timer_update(s_soft_timer_array[s_active_timer_id].duration -
                            prv_soft_timer_update_period());
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
