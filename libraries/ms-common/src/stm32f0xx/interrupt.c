#include "interrupt.h"

#include <stdint.h>
#include <stdlib.h>

#include "gpio.h"
#include "gpio_cfg.h"
#include "status.h"
#include "stm32f0xx_exti.h"
#include "stm32f0xx_misc.h"
#include "stm32f0xx_rcc.h"
#include "stm32f0xx_syscfg.h"

#define NUM_INTERRUPT_GPIO_CHANNELS 3
static InterruptPriority s_interrupt_gpio_priority_map[NUM_INTERRUPT_GPIO_CHANNELS];

void interrupt_init() {
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);

  for (int i = 0; i < NUM_INTERRUPT_GPIO_CHANNELS; i++) {
    s_interrupt_gpio_priority_map[i] = NUM_INTERRUPT_PRIORITY;
  }
}

// Use this to count how many nested disables there are and only enable once out of all critical
// sections.
static uint8_t s_interrupt_priority_mask = 0;

void interrupt_enable() {
  if (s_interrupt_priority_mask > 0) {
    s_interrupt_priority_mask--;
  }
  if (!s_interrupt_priority_mask) {
    __enable_irq();
  }
}

void interrupt_disable() {
  s_interrupt_priority_mask++;
  __disable_irq();
}

// REGISTRATION AND TRIGGER HELPERS
// Use these functions to register and trigger interrupts using module specific registration and
// trigger functions.

#define NUM_INTERRUPT_CHANNELS 32

// Enables the nested interrupt vector controller for a given EXTI line
StatusCode prv_nvic_enable(uint8_t irq_channel, InterruptPriority priority) {
  if (priority >= NUM_INTERRUPT_PRIORITY || irq_channel >= NUM_INTERRUPT_CHANNELS) {
    return status_code(STATUS_CODE_INVALID_ARGS);
  }

  NVIC_InitTypeDef init_struct = {.NVIC_IRQChannel = irq_channel,
                                  .NVIC_IRQChannelPriority = priority,
                                  .NVIC_IRQChannelCmd = ENABLE };

  NVIC_Init(&init_struct);

  return STATUS_CODE_OK;
}

#define NUM_INTERRUPT_LINES 32

// Enables the interrupt line with the given settings.
StatusCode prv_exti_enable(uint8_t line, InterruptSettings *settings) {
  if (line >= NUM_INTERRUPT_LINES || settings->type >= NUM_INTERRUPT_TYPE ||
      settings->edge >= NUM_INTERRUPT_EDGE) {
    return status_code(STATUS_CODE_INVALID_ARGS);
  }

  EXTI_InitTypeDef init_struct = {.EXTI_Line = 0x01 << line,
                                  .EXTI_Mode = 0x04 * settings->type,
                                  .EXTI_Trigger = 0x08 + 0x04 * settings->edge,
                                  .EXTI_LineCmd = ENABLE };
  EXTI_Init(&init_struct);

  return STATUS_CODE_OK;
}

// Trigger a software interrupt on an EXTI line if that line supports this functionality.
StatusCode prv_exti_trigger_interrupt(uint8_t line) {
  // It is not possible to manually trigger interrupts for lines 18 and > 22.
  if (line == 18 || line > 22) {
    return status_code(STATUS_CODE_INVALID_ARGS);
  }

  EXTI_GenerateSWInterrupt(0x01 << line);
  return STATUS_CODE_OK;
}

// Gets the value of the pending flag for a given interrupt. A pending_bit that gets set to 1
// indicates an interrupt is waiting.
StatusCode prv_exti_get_pending(uint8_t line, uint8_t *pending_bit) {
  if (line >= NUM_INTERRUPT_LINES) {
    return status_code(STATUS_CODE_INVALID_ARGS);
  }

  *pending_bit = EXTI_GetITStatus(0x01 << line);
  return STATUS_CODE_OK;
}

// Clears the pending flag for the interrupt.
StatusCode prv_exti_clear_pending(uint8_t line) {
  if (line >= NUM_INTERRUPT_LINES) {
    return status_code(STATUS_CODE_INVALID_ARGS);
  }

  EXTI_ClearITPendingBit(0x01 << line);
  return STATUS_CODE_OK;
}

// GPIO INTERRUPTS

static interrupt_callback s_interrupt_gpio_callback[NUM_GPIO_PINS];

// Helper function to enable gpio interrupts.
StatusCode prv_gpio_interrupt_enable(GPIOAddress *address, InterruptSettings *settings,
                                     interrupt_callback callback, uint8_t irq_channel) {
  SYSCFG_EXTILineConfig(address->port, address->pin);
  status_ok_or_return(prv_exti_enable(address->pin, settings));
  s_interrupt_gpio_callback[address->pin] = callback;
  status_ok_or_return(prv_nvic_enable(irq_channel, settings->priority));
  return STATUS_CODE_OK;
}

StatusCode interrupt_gpio_register(GPIOAddress *address, InterruptSettings *settings,
                                   interrupt_callback callback) {
  if (address->port >= NUM_GPIO_PORTS || address->pin >= NUM_GPIO_PINS) {
    return status_code(STATUS_CODE_INVALID_ARGS);
  } else if (s_interrupt_gpio_callback[address->pin] != NULL) {
    return status_msg(STATUS_CODE_RESOURCE_EXHAUSTED, "Interrupt for this pin is already used.");
  }

  if (address->pin < 2) {
    // Pins 0, 1 are on NVIC channel 5 which is gpio priority group 0.
    if (s_interrupt_gpio_priority_map[0] != settings->priority &&
        s_interrupt_gpio_priority_map[0] != NUM_INTERRUPT_PRIORITY) {
      return status_msg(STATUS_CODE_INVALID_ARGS, "Priority already set for pins 0-1.");
    }
    status_ok_or_return(prv_gpio_interrupt_enable(address, settings, callback, 5));
  } else if (address->pin < 4) {
    // Pins 2, 3 are on NVIC channel 6 which is gpio priority group 1.
    if (s_interrupt_gpio_priority_map[1] != settings->priority &&
        s_interrupt_gpio_priority_map[1] != NUM_INTERRUPT_PRIORITY) {
      return status_msg(STATUS_CODE_INVALID_ARGS, "Priority already set for pins 2-3.");
    }
    status_ok_or_return(prv_gpio_interrupt_enable(address, settings, callback, 6));
  } else {
    // Pins 4, 15 are on NVIC channel 7 which is gpio priority group 2.
    if (s_interrupt_gpio_priority_map[2] != settings->priority &&
        s_interrupt_gpio_priority_map[2] != NUM_INTERRUPT_PRIORITY) {
      return status_msg(STATUS_CODE_INVALID_ARGS, "Priority already set for pins 4-15.");
    }
    status_ok_or_return(prv_gpio_interrupt_enable(address, settings, callback, 7));
  }
  return STATUS_CODE_OK;
}

StatusCode interrupt_gpio_trigger(GPIOAddress *address) {
  if (address->port >= NUM_GPIO_PORTS || address->pin >= NUM_GPIO_PINS) {
    return status_code(STATUS_CODE_INVALID_ARGS);
  }

  prv_exti_trigger_interrupt(address->pin);
  return STATUS_CODE_OK;
}

// Callback runner for GPIO which runs callbacks based on which callbacks are associated with an IRQ
// channel. The function runs the callbacks which have a flag raise in the range [lower_bound,
// upperbound).
void prv_run_gpio_callbacks(uint8_t lower_bound, uint8_t upper_bound) {
  uint8_t pending;
  for (int i = lower_bound; i < upper_bound; i++) {
    prv_exti_get_pending(i, &pending);
    if (pending && s_interrupt_gpio_callback[i] != NULL) {
      s_interrupt_gpio_callback[i]();
    }
    prv_exti_clear_pending(i);
  }
}

// IV Handler for pins 0, 1.
void EXTI0_1_IRQHandler(void) {
  prv_run_gpio_callbacks(0, 2);
}

// IV Handler for pins 2, 3.
void EXTI2_3_IRQHandler(void) {
  prv_run_gpio_callbacks(2, 4);
}

// IV Handler for pins 4 - 15.
void EXTI4_15_IRQHandler(void) {
  prv_run_gpio_callbacks(4, 16);
}
