#include "FreeRTOS.h"

#include "task.h"

#include "blink.h"
#include "blink2.h"

#define INIT_TASK_STACK_SIZE 128

// Statically allocated memory for producer_task stack
static StackType_t s_ProducerTaskStack[INIT_TASK_STACK_SIZE];

// Statically allocated memory for producer_task task control block
static StaticTask_t s_ProducerTaskTCB;

StackType_t *init_get_stack(void) {
  return &s_ProducerTaskStack[0];
}

StaticTask_t *init_get_tcb(void) {
  return &s_ProducerTaskTCB;
}

void init_task(void *params) {
  // Setup the task
  xTaskCreateStatic(blink_task, "Blink LED", configMINIMAL_STACK_SIZE, NULL, BLINK_TASK_PRIORITY,
                    blink_get_stack(), blink_get_tcb());

  xTaskCreateStatic(blink_2_task, "Blink LED 2", configMINIMAL_STACK_SIZE, NULL, BLINK_TASK_PRIORITY,
                    blink_2_get_stack(), blink_2_get_tcb());

  vTaskDelete(NULL);
}

int main() {
  xTaskCreateStatic(init_task, "Init Task", configMINIMAL_STACK_SIZE, NULL, BLINK_TASK_PRIORITY,
                    init_get_stack(), init_get_tcb());
  /// Start the scheduler
  vTaskStartScheduler();

  for (;;) {
    // Infinite loop
  }
  /// Will only get here if there was insufficient heap to start the scheduler.
  return 0;
}
