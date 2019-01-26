#include "FreeRTOS.h"

#include "task.h"

#include "blink.h"

int main() {
  xTaskCreateStatic(blink_task, "Blink LED", 128, NULL, BLINK_TASK_PRIORITY, blink_get_stack(),
                    blink_get_tcb());

  // Start the scheduler
  vTaskStartScheduler();
  // Will only get here if the scheduler failed to start.

  return 0;
}
