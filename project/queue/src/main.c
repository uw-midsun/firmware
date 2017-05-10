#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include "event_queue.h"

int main(void) {
  event_queue_init();

  event_raise(&((Event){ .id = 40, .data = 0 }));
  event_raise(&((Event){ .id = 20, .data = 1 }));
  event_raise(&((Event){ .id = 60, .data = 3 }));

  Event e;
  while (event_process(&e)) {
    printf("id: %d data: %d\n", e.id, e.data);
  }

  return 0;
}
