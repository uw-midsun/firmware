#include "gpio_it.h"

#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>

#include "gpio.h"
#include "interrupt_def.h"
#include "status.h"
#include "x86_interrupt.h"

typedef struct GPIOITInterrupt {
  uint8_t interrupt_id;
  GPIOAddress address;
  InterruptEdge edge;
  gpio_it_callback callback;
  void *context;
} GPIOITInterrupt;

static uint8_t s_gpio_it_handler_id;
static GPIOITInterrupt s_gpio_it_interrupts[GPIO_PINS_PER_PORT];

static void prv_gpio_it_handler(uint8_t interrupt_id) {
  GPIOState state;

  for (int i = 0; i < GPIO_PINS_PER_PORT; i++) {
    // Check if the change in value corresponds with the specified edge trigger
    gpio_get_value(&s_gpio_it_interrupts[i].address, &state);
    bool edge_fail = ((s_gpio_it_interrupts[i].edge == INTERRUPT_EDGE_RISING) &&
      (state != GPIO_STATE_HIGH)) || ((s_gpio_it_interrupts[i].edge == INTERRUPT_EDGE_FALLING) &&
      (state != GPIO_STATE_LOW));

    if (s_gpio_it_interrupts[i].interrupt_id == interrupt_id &&
        s_gpio_it_interrupts[i].callback != NULL && !edge_fail) {
      s_gpio_it_interrupts[i].callback(&s_gpio_it_interrupts[i].address,
                                       s_gpio_it_interrupts[i].context);
    }
  }
}

void gpio_it_init(void) {
  x86_interrupt_register_handler(prv_gpio_it_handler, &s_gpio_it_handler_id);

  GPIOITInterrupt empty_cfg = { 0 };
  for (uint16_t i = 0; i < GPIO_PINS_PER_PORT; i++) {
    s_gpio_it_interrupts[i] = empty_cfg;
  }
}

StatusCode gpio_it_register_interrupt(const GPIOAddress *address, const InterruptSettings *settings,
                                      InterruptEdge edge, gpio_it_callback callback,
                                      void *context) {
  if (address->port >= NUM_GPIO_PORTS || address->pin >= GPIO_PINS_PER_PORT) {
    return status_code(STATUS_CODE_INVALID_ARGS);
  } else if (s_gpio_it_interrupts[address->pin].callback) {
    return status_msg(STATUS_CODE_RESOURCE_EXHAUSTED, "Pin already in use.");
  }

  uint8_t interrupt_id;
  status_ok_or_return(
      x86_interrupt_register_interrupt(s_gpio_it_handler_id, settings, &interrupt_id));

  s_gpio_it_interrupts[address->pin].interrupt_id = interrupt_id;
  s_gpio_it_interrupts[address->pin].address = *address;
  s_gpio_it_interrupts[address->pin].edge = edge;
  s_gpio_it_interrupts[address->pin].callback = callback;
  s_gpio_it_interrupts[address->pin].context = context;

  return STATUS_CODE_OK;
}

StatusCode gpio_it_trigger_interrupt(const GPIOAddress *address) {
  if (address->port >= NUM_GPIO_PORTS || address->pin >= GPIO_PINS_PER_PORT) {
    return status_code(STATUS_CODE_INVALID_ARGS);
  }

  return x86_interrupt_trigger(s_gpio_it_interrupts[address->pin].interrupt_id);
}
