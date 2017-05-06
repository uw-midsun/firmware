#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include "event_queue.h"
#include "stm32f0xx.h"

int main(void) {
  event_queue_init();

  event_raise(&((Event){ .id = 40, .data = 0 }));
  event_raise(&((Event){ .id = 20, .data = 1 }));
  event_raise(&((Event){ .id = 60, .data = 3 }));

  Event e;
  while (event_process(&e)) {
    printf("id: %d data: %d\n", e.id, e.data);
  }

  volatile RCC_ClocksTypeDef clk = { 0 };
  RCC_GetClocksFreq(&clk);

  printf("SYSCLK: %d\n", clk.SYSCLK_Frequency);

  while (true) {
    printf("0123\n");

    for (volatile int i = 0; i < 8000000; i++) { }
  }

  __asm("BKPT #0");

  return 0;
}
