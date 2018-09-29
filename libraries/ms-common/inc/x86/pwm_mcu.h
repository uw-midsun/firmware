#pragma once

typedef enum {
  PWM_TIMER_1 = 0,
  // PWM_TIMER_2,  // Requisitioned to back the soft_timer module.
  PWM_TIMER_3,
  PWM_TIMER_14,
  PWM_TIMER_15,
  PWM_TIMER_16,
  PWM_TIMER_17,
  NUM_PWM_TIMERS,
} PWMTimer;
