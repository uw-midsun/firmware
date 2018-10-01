#include "FreeRTOS.h"

#include "task.h"

#include "blink.h"
#include "blink2.h"

int main() {
  xTaskCreateStatic(blink_task, "Blink LED", 128, NULL, BLINK_TASK_PRIORITY, blink_get_stack(),
                    blink_get_tcb());

  xTaskCreateStatic(blink_2_task, "Blink LED 2", BLINK_2_TASK_STACK_SIZE, NULL,
                    BLINK_2_TASK_PRIORITY, blink_2_get_stack(), blink_2_get_tcb());

  // Start the scheduler
  vTaskStartScheduler();
  // Will only get here if the scheduler failed to start.

  return 0;
}
