#include "soft_timer.h"

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#include "status.h"
#include "stm32f0xx_rcc.h"
#include "stm32f0xx_tim.h"

typedef struct timer {
  uint32_t duration;
  uint32_t time_started;
  uint32_t last_update_value;
  void *context;
  bool inuse;
  bool rolled_over;
  SoftTimerCallback callback;
} timer;

static timer s_soft_timer_array[SOFT_TIMER_MAX_TIMERS];

// Used to track rollovers this will happen every ~1.19 hrs as we are using microseconds.

void soft_timer_init(void) {
  for (uint32_t i = 0; i < SOFT_TIMER_MAX_TIMERS; i++) {
    s_soft_timer_array[i].inuse = false;
    s_soft_timer_array[i].last_update_value = 0;
  }

  RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);

  RCC_ClocksTypeDef clock_speeds;
  RCC_GetClocksFreq(&clock_speeds);

  // Configure each clock tick to be 1 microsecond from 0 to UINT32_MAX. Uses the external 8 MHz
  // clock for tick source. This will need to adjusted or made into a variable if we choose a
  // different external clock from the discovery board.
  TIM_TimeBaseInitTypeDef init_struct = {.TIM_Prescaler = clock_speeds.SYSCLK_Frequency / 1000000,
                                         .TIM_CounterMode = TIM_CounterMode_Up,
                                         .TIM_Period = UINT32_MAX,
                                         .TIM_ClockDivision = TIM_CKD_DIV1 };

  TIM_TimeBaseInit(TIM2, &init_struct);
  TIM_Cmd(TIM2, ENABLE);
}

StatusCode soft_timer_start(uint32_t duration_us, SoftTimerCallback callback, void *context,
                            SoftTimerID *timer_id) {
  uint32_t curr_time = TIM_GetCounter(TIM2);
  for (uint32_t i = 0; i < SOFT_TIMER_MAX_TIMERS; i++) {
    if (!s_soft_timer_array[i].inuse) {
      // Populate next available timer if possible.
      s_soft_timer_array[i].duration = duration_us;
      s_soft_timer_array[i].time_started = curr_time;
      s_soft_timer_array[i].last_update_value = curr_time;
      s_soft_timer_array[i].callback = callback;
      s_soft_timer_array[i].context = context;
      s_soft_timer_array[i].inuse = true;
      s_soft_timer_array[i].rolled_over = false;
      *timer_id = i;
      return STATUS_CODE_OK;
    }
  }

  return status_msg(STATUS_CODE_RESOURCE_EXHAUSTED, "Out of software timers.");
}

void soft_timer_update(void) {
  // Get the time to use to update.
  uint32_t curr_time = TIM_GetCounter(TIM2);

  for (uint32_t i = 0; i < SOFT_TIMER_MAX_TIMERS; i++) {
    if (s_soft_timer_array[i].inuse) {
      // For each inuse timer see if the timer expired. If it has run its callback. Otherwise update
      // if it has rolled over.

      // If the timer has rolled over relative to the last time it was updated take note of this.
      bool rolled_over = false;
      if (s_soft_timer_array[i].last_update_value > curr_time) {
        rolled_over = true;
      }

      if ((uint64_t)curr_time + UINT32_MAX * rolled_over - s_soft_timer_array[i].time_started >
          s_soft_timer_array[i].duration) {
        s_soft_timer_array[i].callback(i, s_soft_timer_array[i].context);
        s_soft_timer_array[i].inuse = false;
        s_soft_timer_array[i].last_update_value = 0;
      } else {
        s_soft_timer_array[i].last_update_value = curr_time;
        s_soft_timer_array[i].rolled_over = rolled_over;
      }
    }
  }
}

bool soft_timer_inuse(void) {
  for (uint32_t i = 0; i < SOFT_TIMER_MAX_TIMERS; i++) {
    if (s_soft_timer_array[i].inuse) {
      return true;
    }
  }

  return false;
}
