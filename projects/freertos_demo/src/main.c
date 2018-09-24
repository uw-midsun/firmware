#include "FreeRTOS.h"

#include "task.h"

#include "blink.h"
#include "blink2.h"

/* configSUPPORT_STATIC_ALLOCATION is set to 1, so the application must provide an
implementation of vApplicationGetIdleTaskMemory() to provide the memory that is
used by the Idle task. */
void vApplicationGetIdleTaskMemory(StaticTask_t **ppxIdleTaskTCBBuffer,
                                   StackType_t **ppxIdleTaskStackBuffer,
                                   uint32_t *pulIdleTaskStackSize) {
  /* If the buffers to be provided to the Idle task are declared inside this
  function then they must be declared static - otherwise they will be allocated on
  the stack and so not exists after this function exits. */
  static StaticTask_t xIdleTaskTCB;
  static StackType_t uxIdleTaskStack[configMINIMAL_STACK_SIZE];

  /* Pass out a pointer to the StaticTask_t structure in which the Idle task's
  state will be stored. */
  *ppxIdleTaskTCBBuffer = &xIdleTaskTCB;

  /* Pass out the array that will be used as the Idle task's stack. */
  *ppxIdleTaskStackBuffer = uxIdleTaskStack;

  /* Pass out the size of the array pointed to by *ppxIdleTaskStackBuffer.
  Note that, as the array is necessarily of type StackType_t,
  configMINIMAL_STACK_SIZE is specified in words, not bytes. */
  *pulIdleTaskStackSize = configMINIMAL_STACK_SIZE;
}

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
