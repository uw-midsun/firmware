#include "delay.h"

#include <stdbool.h>

#include "soft_timer.h"
#include "wait.h"

static void prv_delay_it(SoftTimerID timer_id, void* context) {
  bool* block = context;
  *block = false;
}

void delay_us(uint32_t t) {
  volatile bool block = true;
  SoftTimerID id;
  soft_timer_start(t, prv_delay_it, &block, &id);
  while (block) {
    wait();
  }
}
