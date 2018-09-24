#pragma once
// An example blinker task

#define BLINK_TASK_PRIORITY 1

// Fetch the task's stack
StackType_t *blink_2_get_stack(void);

// Fetch the Task Control Block
StaticTask_t *blink_2_get_tcb(void);

// Blinker task
void blink_2_task(void *params);
