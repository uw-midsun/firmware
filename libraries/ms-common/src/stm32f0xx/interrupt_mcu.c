#include "interrupt_mcu.h"

#include <stdbool.h>
#include <stdint.h>

#include "interrupt.h"
#include "status.h"
#include "stm32f0xx_exti.h"
#include "stm32f0xx_misc.h"
#include "stm32f0xx_rcc.h"
#include "stm32f0xx_syscfg.h"

// Channels are NVIC channels which handle prioritization of interupts.
#define NUM_INTERRUPT_MCU_CHANNELS 32
// Lines are external interrupt channels often grouped to a single
#define NUM_INTERRUPT_MCU_LINES 32

static InterruptPriority s_interrupt_mcu_priorities[NUM_INTERRUPT_MCU_CHANNELS];
static bool s_interrupt_mcu_initialized = false;

void interrupt_mcu_init(void) {
  if (!s_interrupt_mcu_initialized) {
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);

    for (int i = 0; i < NUM_INTERRUPT_MCU_CHANNELS; i++) {
      s_interrupt_mcu_priorities[i] = NUM_INTERRUPT_PRIORITY;
    }
    s_interrupt_mcu_initialized = true;
  }
}

StatusCode interrupt_mcu_nvic_enable(uint8_t irq_channel, InterruptPriority priority) {
  if (priority >= NUM_INTERRUPT_PRIORITY || irq_channel >= NUM_INTERRUPT_MCU_CHANNELS) {
    return status_code(STATUS_CODE_INVALID_ARGS);
  } else if (s_interrupt_mcu_priorities[irq_channel] == priority) {
    return STATUS_CODE_OK;
  } else if (s_interrupt_mcu_priorities[irq_channel] != NUM_INTERRUPT_PRIORITY) {
    return status_msg(STATUS_CODE_RESOURCE_EXHAUSTED, "Priority already set.");
  }

  NVIC_InitTypeDef init_struct = { .NVIC_IRQChannel = irq_channel,
                                   .NVIC_IRQChannelPriority = priority,
                                   .NVIC_IRQChannelCmd = ENABLE };

  NVIC_Init(&init_struct);

  return STATUS_CODE_OK;
}

StatusCode interrupt_mcu_exti_enable(uint8_t line, InterruptSettings *settings,
                                     InterruptEdge edge) {
  if (line >= NUM_INTERRUPT_MCU_LINES || settings->type >= NUM_INTERRUPT_TYPE ||
      edge >= NUM_INTERRUPT_EDGE) {
    return status_code(STATUS_CODE_INVALID_ARGS);
  }

  EXTI_InitTypeDef init_struct = { .EXTI_Line = 0x01 << line,
                                   .EXTI_Mode = 0x04 * settings->type,
                                   .EXTI_Trigger = 0x08 + 0x04 * edge,
                                   .EXTI_LineCmd = ENABLE };
  EXTI_Init(&init_struct);

  return STATUS_CODE_OK;
}

StatusCode interrupt_mcu_exti_trigger(uint8_t line) {
  // It is not possible to manually trigger interrupts for lines 18 and > 22.
  if (line == 18 || line > 22) {
    return status_code(STATUS_CODE_INVALID_ARGS);
  }

  EXTI_GenerateSWInterrupt(0x01 << line);
  return STATUS_CODE_OK;
}

StatusCode interrupt_mcu_exti_get_pending(uint8_t line, uint8_t *pending_bit) {
  if (line >= NUM_INTERRUPT_MCU_LINES) {
    return status_code(STATUS_CODE_INVALID_ARGS);
  }

  *pending_bit = (uint8_t)EXTI_GetITStatus(0x01 << line);
  return STATUS_CODE_OK;
}

StatusCode interrupt_mcu_exti_clear_pending(uint8_t line) {
  if (line >= NUM_INTERRUPT_MCU_LINES) {
    return status_code(STATUS_CODE_INVALID_ARGS);
  }

  EXTI_ClearITPendingBit(0x01 << line);
  return STATUS_CODE_OK;
}
