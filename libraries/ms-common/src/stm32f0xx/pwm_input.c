#include "pwm_input.h"

#include <stdint.h>
#include <string.h>

#include "critical_section.h"
#include "gpio.h"
#include "log.h"
#include "pwm.h"
#include "stm32f0xx.h"

static void prv_input_handle_interrupt(PwmTimer timer);

// These IRQHandlers may need to be freed in the future
// so that others can use them.
void TIM1_CC_IRQHandler(void) {
  prv_input_handle_interrupt(PWM_TIMER_1);
}

void TIM3_IRQHandler(void) {
  prv_input_handle_interrupt(PWM_TIMER_3);
}

void TIM14_IRQHandler(void) {
  prv_input_handle_interrupt(PWM_TIMER_14);
}

void TIM15_IRQHandler(void) {
  prv_input_handle_interrupt(PWM_TIMER_15);
}

void TIM16_IRQHandler(void) {
  prv_input_handle_interrupt(PWM_TIMER_16);
}

void TIM17_IRQHandler(void) {
  prv_input_handle_interrupt(PWM_TIMER_17);
}

typedef struct {
  void (*rcc_cmd)(uint32_t periph, FunctionalState state);
  uint32_t periph;
  uint32_t irq;
  volatile uint32_t dc;
  volatile uint32_t period;
  TIM_TypeDef *base;
  PwmInputStorage *storage;
  uint16_t channel;
} PwmTimerData;

static PwmTimerData s_port[] = {
  [PWM_TIMER_1] = { .rcc_cmd = RCC_APB2PeriphClockCmd,
                    .periph = RCC_APB2Periph_TIM1,
                    .irq = TIM1_CC_IRQn,
                    .base = TIM1 },
  [PWM_TIMER_3] = { .rcc_cmd = RCC_APB1PeriphClockCmd,
                    .periph = RCC_APB1Periph_TIM3,
                    .irq = TIM3_IRQn,
                    .base = TIM3 },
  [PWM_TIMER_14] = { .rcc_cmd = RCC_APB1PeriphClockCmd,
                     .periph = RCC_APB1Periph_TIM14,
                     .irq = TIM14_IRQn,
                     .base = TIM14 },
  [PWM_TIMER_15] = { .rcc_cmd = RCC_APB2PeriphClockCmd,
                     .periph = RCC_APB2Periph_TIM15,
                     .irq = TIM15_IRQn,
                     .base = TIM15 },
  [PWM_TIMER_16] = { .rcc_cmd = RCC_APB2PeriphClockCmd,
                     .periph = RCC_APB2Periph_TIM16,
                     .irq = TIM16_IRQn,
                     .base = TIM16 },
  [PWM_TIMER_17] = { .rcc_cmd = RCC_APB2PeriphClockCmd,
                     .periph = RCC_APB2Periph_TIM17,
                     .irq = TIM17_IRQn,
                     .base = TIM17 },
};

static void prv_input_handle_interrupt(PwmTimer timer) {
  TIM_TypeDef *tim_location = s_port[timer].base;

  if (TIM_GetITStatus(tim_location, TIM_IT_Update) == SET) {
    TIM_ClearITPendingBit(tim_location, TIM_IT_Update);
  } else if (TIM_GetITStatus(tim_location, TIM_IT_CC1) == RESET) {
    return;
  }

  TIM_ClearITPendingBit(tim_location, TIM_IT_CC1);

  uint32_t IC2Value_1 = 0;
  uint32_t IC2Value_2 = 0;

  uint32_t period = 0;
  uint32_t dc = 0;

  if (s_port[timer].channel == TIM_Channel_2) {
    IC2Value_1 = TIM_GetCapture2(tim_location);
    IC2Value_2 = TIM_GetCapture1(tim_location);
  } else {
    IC2Value_2 = TIM_GetCapture2(tim_location);
    IC2Value_1 = TIM_GetCapture1(tim_location);
  }

  // LOG_DEBUG("Period candidate: %d\n", (int) IC2Value_1);

  if (IC2Value_1 != 0) {
    dc = (IC2Value_2 * 1000) / IC2Value_1;
    period = IC2Value_1;
  } else {
    dc = 0;
    period = 0;
  }

  s_port[timer].period = period;
  s_port[timer].dc = dc;

  if (s_port[timer].storage->callback != NULL) {
    s_port[timer].storage->callback(dc, period, s_port[timer].storage->context);
  }
}

StatusCode pwm_input_init(PwmTimer timer, PwmInputSettings *settings, PwmInputStorage *storage) {
  s_port[timer].rcc_cmd(s_port[timer].periph, ENABLE);

  memset(storage, 0, sizeof(*storage));
  s_port[timer].storage = storage;
  s_port[timer].storage->callback = settings->callback;
  s_port[timer].storage->context = settings->context;
  s_port[timer].channel = settings->channel;

  uint16_t trigger_source = 0;

  if (settings->channel == PWM_CHANNEL_1) {
    s_port[timer].channel = TIM_Channel_1;
    trigger_source = TIM_TS_TI1FP1;
  } else {
    s_port[timer].channel = TIM_Channel_2;
    trigger_source = TIM_TS_TI2FP2;
  }

  stm32f0xx_interrupt_nvic_enable(s_port[timer].irq, 0);

  TIM_TypeDef *tim_location = s_port[timer].base;

  RCC_ClocksTypeDef clocks;
  RCC_GetClocksFreq(&clocks);

  TIM_TimeBaseInitTypeDef tim_init = {
    .TIM_Prescaler = (clocks.PCLK_Frequency / 1000000) - 1,
    .TIM_CounterMode = TIM_CounterMode_Up,
    .TIM_Period = 0xFFFFFFFF,
    .TIM_ClockDivision = TIM_CKD_DIV1,
    .TIM_RepetitionCounter = 0,
  };

  TIM_TimeBaseInit(tim_location, &tim_init);

  TIM_ICInitTypeDef tim_icinit = {
    .TIM_Channel = s_port[timer].channel,
    .TIM_ICPolarity = TIM_ICPolarity_Rising,
    .TIM_ICSelection = TIM_ICSelection_DirectTI,
    .TIM_ICPrescaler = TIM_ICPSC_DIV1,
    .TIM_ICFilter = 0x0,
  };

  TIM_PWMIConfig(tim_location, &tim_icinit);

  TIM_SelectInputTrigger(tim_location, trigger_source);
  TIM_SelectSlaveMode(tim_location, TIM_SlaveMode_Reset);
  TIM_SelectMasterSlaveMode(tim_location, TIM_MasterSlaveMode_Enable);
  TIM_SelectOutputTrigger(tim_location, TIM_TRGOSource_Reset);
  TIM_Cmd(tim_location, ENABLE);
  TIM_ITConfig(tim_location, TIM_IT_CC1, ENABLE);

  return STATUS_CODE_OK;
}

uint32_t pwm_input_get_period(PwmTimer timer) {
  CRITICAL_SECTION_AUTOEND;
  uint32_t result = s_port[timer].period;
  s_port[timer].period = 0;
  return result;
}

uint32_t pwm_input_get_dc(PwmTimer timer) {
  CRITICAL_SECTION_AUTOEND;
  uint32_t result = s_port[timer].dc;

  if (result == 0) {
    if (s_port[timer].storage->callback != NULL) {
      s_port[timer].storage->callback(result, pwm_input_get_period(timer),
                                      s_port[timer].storage->context);
    }
  }

  s_port[timer].dc = 0;
  return result;
}
