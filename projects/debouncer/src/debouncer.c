#include "debouncer.h"

#include "log.h"
#include "soft_timer.h"
#include "stm32f0xx_interrupt.h"

static void prv_debouncer_timer_callback(SoftTimerID timer_id, void *context) {
  DebounceInfo *debounce_info = context;

  GPIOState current_state;
  gpio_get_state(&debounce_info->address, &current_state);

  if (current_state == debounce_info->state) {
    debounce_info->callback(&debounce_info->address, debounce_info->context);
  }

  stm32f0xx_interrupt_exti_mask_set(0, false);
}

// this function should be registered as the interrupt callback on the given address
static void prv_debouncer_it_callback(const GPIOAddress *address, void *context) {
  DebounceInfo *debounce_info = context;
  gpio_get_state(address, &debounce_info->state);

  stm32f0xx_interrupt_exti_mask_set(0, true);

  soft_timer_start_millis(50, prv_debouncer_timer_callback, debounce_info, NULL);
}

StatusCode debouncer_init_pin(DebounceInfo *debounce_info, const GPIOAddress *address,
                              gpio_it_callback callback, void *context) {
  GPIOSettings gpio_settings = {
    .direction = GPIO_DIR_IN,   //
    .resistor = GPIO_RES_NONE,  //
  };

  gpio_init_pin(address, &gpio_settings);

  InterruptSettings interrupt_settings = {
    .type = INTERRUPT_TYPE_INTERRUPT,
    .priority = INTERRUPT_PRIORITY_LOW,
  };

  debounce_info->address = *address;
  debounce_info->callback = callback;
  debounce_info->context = context;

  StatusCode ret =
      gpio_it_register_interrupt(address, &interrupt_settings, INTERRUPT_EDGE_RISING_FALLING,
                                 prv_debouncer_it_callback, debounce_info);
  return ret;
}
