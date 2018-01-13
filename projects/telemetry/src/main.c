#include "gpio.h"  // General Purpose I/O control.
#include "interrupt.h"   // For enabling interrupts.
#include "soft_timer.h"  // Software timers for scheduling future events.

int main(void) {
  // Enable various peripherals
  interrupt_init();
  soft_timer_init();
  gpio_init();

  return 0;
}
