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
  soft_timer_callback callback;
  bool inuse;
  bool rolled_over;
  void *context;
} timer;

static timer s_soft_timer_array[SOFT_TIMER_NUM];

// Used to track rollovers this will happen every ~1.19 hrs as we are using microseconds.
static uint32_t s_soft_timer_last_value = 0;
static bool s_soft_timer_initialized = false;

void soft_timer_init(uint8_t clock_speed) {
  if (!s_soft_timer_initialized) {
    for (uint8_t i = 0; i < SOFT_TIMER_NUM; i++) {
      s_soft_timer_array[i].inuse = false;
    }

    // Configure each clock tick to be 1 microsecond from 0 to UINT32_MAX. Uses the external 8 MHz
    // clock for tick source. This will need to adjusted or made into a variable if we choose a
    // different external clock from the discovery board.
    TIM_TimeBaseInitTypeDef init_struct = {.TIM_Prescaler = clock_speed,
                                           .TIM_CounterMode = TIM_CounterMode_Up,
                                           .TIM_Period = UINT32_MAX,
                                           .TIM_ClockDivision = TIM_CKD_DIV1 };

    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);
    TIM_TimeBaseInit(TIM2, &init_struct);
    TIM_Cmd(TIM2, ENABLE);

    s_soft_timer_initialized = true;
  }
}

StatusCode soft_timer_start(uint32_t duration_micros, soft_timer_callback callback,
                            uint8_t *timer_id, void *context) {
  uint32_t curr_time = TIM_GetCounter(TIM2);
  for (uint8_t i = 0; i < SOFT_TIMER_NUM; i++) {
    if (!s_soft_timer_array[i].inuse) {
      // Populate next available timer if possible.
      s_soft_timer_array[i].duration = duration_micros;
      s_soft_timer_array[i].time_started = curr_time;
      s_soft_timer_array[i].callback = callback;
      s_soft_timer_array[i].context = context;
      s_soft_timer_array[i].inuse = true;
      s_soft_timer_array[i].rolled_over = false;
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
  // Get the time to use to update.
  uint32_t tmp_last_time = s_soft_timer_last_value;
  s_soft_timer_last_value = TIM_GetCounter(TIM2);

  // If the timer has rolled over relative to the last time it was update take note of this.
  bool rolled_over = false;
  if (tmp_last_time > s_soft_timer_last_value) {
    rolled_over = true;
  }

  for (uint8_t i = 0; i < SOFT_TIMER_NUM; i++) {
    if (s_soft_timer_array[i].inuse) {
      // For each inuse timer see if the timer expired. If it has run its callback. Otherwise update
      // if it has rolled over.
      if ((uint64_t)s_soft_timer_last_value + UINT32_MAX * rolled_over -
              s_soft_timer_array[i].time_started >
          s_soft_timer_array[i].duration) {
        s_soft_timer_array[i].callback(i, s_soft_timer_array[i].context);
        s_soft_timer_array[i].inuse = false;
      } else {
        s_soft_timer_array[i].rolled_over = rolled_over;
      }
    }
  }
}
