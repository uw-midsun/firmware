#include "charger_pin.h"

#include <stddef.h>

#include "charger_events.h"
#include "event_queue.h"
#include "gpio.h"
#include "gpio_it.h"
#include "interrupt_def.h"
#include "status.h"

static void prv_charger_pin_it(const GpioAddress *address, void *context) {
  (void)context;
  GPIOState state = NUM_GPIO_STATES;
  gpio_get_state(address, &state);
  switch (state) {
    // TODO(ELEC-355): Determine if this logic is correct.
    case GPIO_STATE_LOW:
      event_raise(CHARGER_EVENT_DISCONNECTED, 0);
      break;
    case GPIO_STATE_HIGH:
      event_raise(CHARGER_EVENT_CONNECTED, 0);
    case NUM_GPIO_STATES:
    default:
      // Unreachable
      break;
  }
}

StatusCode charger_pin_init(const GpioAddress *address) {
  const GPIOSettings settings = {
    .state = GPIO_STATE_LOW,
    .direction = GPIO_DIR_IN,
    .resistor = GPIO_RES_NONE,
    .alt_function = GPIO_ALTFN_NONE,
  };
  status_ok_or_return(gpio_init_pin(address, &settings));

  const InterruptSettings it_settings = {
    .type = INTERRUPT_TYPE_INTERRUPT,
    .priority = INTERRUPT_PRIORITY_NORMAL,
  };
  status_ok_or_return(gpio_it_register_interrupt(
      address, &it_settings, INTERRUPT_EDGE_RISING_FALLING, prv_charger_pin_it, NULL));

  // Manually query to trigger the starting state.
  prv_charger_pin_it(address, NULL);
  return STATUS_CODE_OK;
}
