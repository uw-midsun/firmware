#include <stdint.h>

#include "stm32f0xx_tim.h"

void TEST_SOFT_TIMER_SET_COUNTER(uint32_t counter_value) {
  TIM_SetCounter(TIM2, counter_value);
  return;
}
