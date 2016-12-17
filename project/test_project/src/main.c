#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "gpio.h"
#include "interrupt.h"
#include "status.h"

static GPIOAddress s_addr[] = { { 2, 6 }, { 2, 7 }, { 2, 8 }, { 2, 9 } };
static const uint32_t s_addr_size = sizeof(s_addr) / sizeof(GPIOAddress);

static volatile uint32_t s_interval = 0;

static void s_gpio_toggle_callback() {
  BEGIN_CRITICAL_SECTION();
  s_interval = 0;
  END_CRITICAL_SECTION();
}

static void s_status_callback(const Status* status) {
  printf("CODE:%d:%s:%s: %s\n", status->code, status->source, status->caller, status->message);
}

int main(void) {
  // Register a debugging callback.
  status_register_callback(s_status_callback);

  // Enable gpio.
  gpio_init();

  // Enable the LEDs as outputs.
  GPIOSettings settings = { GPIO_DIR_OUT, GPIO_STATE_HIGH, GPIO_RES_NONE, GPIO_ALTFN_NONE };
  volatile uint32_t i;
  for (i = 0; i < s_addr_size; i++) {
    gpio_init_pin(&s_addr[i], &settings);
  }

  // Enable the push button as an input.
  GPIOAddress push_addr = { 0, 0 };
  GPIOSettings push = { GPIO_DIR_IN, GPIO_STATE_LOW, GPIO_RES_PULLUP, GPIO_ALTFN_NONE };
  gpio_init_pin(&push_addr, &push);

  // Turn the push button into an interrupt.
  interrupt_init();
  InterruptSettings it_settings = {.type = INTERRUPT_TYPE_INTERRUPT,
                                   .edge = INTERRUPT_EDGE_RISING,
                                   .priority = INTERRUPT_PRIORITY_NORMAL };
  interrupt_gpio_register(&push_addr, &it_settings, s_gpio_toggle_callback);

  // Busy wait for the interrupt.
  while (true) {
    // This is safe to interrupt because it will finish toggling even if interrupted. This is
    // finished before the delay is triggered so there is no noticable effect.
    for (uint32_t j = 0; j < s_addr_size; j++) {
      gpio_toggle_state(&s_addr[j]);
    }

    // Delay. This is also safe to interrupt because if interrupted it will just exit.
    for (uint32_t i = 0; i < s_interval; i++) {
    }

    status_msg(STATUS_CODE_UNKNOWN, "I'm cool");

    s_interval += 10;
  }
}
