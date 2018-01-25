#include "debouncer.h"

#include "log.h"
#include "soft_timer.h"
#include "stm32f0xx_interrupt.h"

#define INTERRUPT_MASKING_DURATION_MS 50

// This is the callback for the soft timer. Gets the state and compares with old state.
// If states are equal, it means that we had a button input. So it runs the given callback.
static void prv_debouncer_timer_callback(SoftTimerID timer_id, void *context) {
  DebouncerInfo *debouncer_info = context;

  GPIOState current_state;
  gpio_get_state(&debouncer_info->address, &current_state);

  if (current_state == debouncer_info->state) {
    debouncer_info->callback(&debouncer_info->address, debouncer_info->context);
  }

  // stm32f0xx_interrupt_exti_mask_set(debouncer_info->address.pin, false);
  gpio_it_mask_interrupt(&debouncer_info->address, false);
}

// Saves the state of gpio, masks the interrupt, and creates a soft timer
static void prv_debouncer_it_callback(const GPIOAddress *address, void *context) {
  DebouncerInfo *debouncer_info = context;
  gpio_get_state(address, &debouncer_info->state);

  gpio_it_mask_interrupt(address, true);

  soft_timer_start_millis(INTERRUPT_MASKING_DURATION_MS, prv_debouncer_timer_callback,
                          debouncer_info, NULL);
}

StatusCode debouncer_init_pin(DebouncerInfo *debouncer_info, const GPIOAddress *address,
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

  debouncer_info->address = *address;
  debouncer_info->callback = callback;
  debouncer_info->context = context;

  StatusCode ret =
      gpio_it_register_interrupt(address, &interrupt_settings, INTERRUPT_EDGE_RISING_FALLING,
                                 prv_debouncer_it_callback, debouncer_info);
  return ret;
}
