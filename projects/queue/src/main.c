#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "event_queue.h"

int main(void) {
  event_queue_init();

  event_raise(40, 0);
  event_raise(20, 1);
  event_raise(60, 3);

  Event e;
  while (event_process(&e) == STATUS_CODE_OK) {
    printf("id: %d data: %d\n", e.id, e.data);
  }

  return 0;
}
