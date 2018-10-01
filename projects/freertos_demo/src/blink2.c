#include <stdbool.h>

#include "FreeRTOS.h"
#include "task.h"

#include "blink2.h"

#include "gpio.h"
#include "misc.h"

#include "ms_task.h"

// Statically allocated memory for producer_task stack
static StackType_t s_task_stack[BLINK_2_TASK_STACK_SIZE];

// Statically allocated memory for producer_task task control block
static StaticTask_t s_task_tcb;

StackType_t *blink_2_get_stack(void) {
  return &s_task_stack[0];
}

StaticTask_t *blink_2_get_tcb(void) {
  return &s_task_tcb;
}

void blink_2_task(void *params) {
  // Initialise so that the first call to vTaskDelayUntil() works correctly
  TickType_t last_execution_time = xTaskGetTickCount();

  const GpioSettings led_settings = {
    .direction = GPIO_DIR_OUT,        // The pin needs to output.
    .state = GPIO_STATE_HIGH,         // Start in the "on" state.
    .alt_function = GPIO_ALTFN_NONE,  // No connections to peripherals.
    .resistor = GPIO_RES_NONE,        // No need of a resistor to modify floating logic levels.
  };

  const GpioAddress leds[] = {
    { .port = GPIO_PORT_B, .pin = 3 },   //
    { .port = GPIO_PORT_A, .pin = 15 },  //
  };

  // Initialize LED GPIOs
  for (size_t i = 0; i < SIZEOF_ARRAY(leds); i++) {
    gpio_init_pin(&leds[i], &led_settings);
  }

  while (true) {
    for (size_t i = 0; i < SIZEOF_ARRAY(leds); i++) {
      gpio_toggle_state(&leds[i]);

      vTaskDelayUntil(&last_execution_time, pdMS_TO_TICKS(FREERTOS_TASK_RATE_10_HZ));
    }
  }
}
