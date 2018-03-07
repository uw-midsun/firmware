#include "charger_pin.h"

#include <stddef.h>

#include "event_queue.h"
#include "gpio.h"
#include "gpio_it.h"
#include "interrupt_def.h"
#include "status.h"

static void prv_charger_pin_it(const GPIOAddress *address, void *context) {
  (void)context;
  GPIOState state = NUM_GPIO_STATES;
  gpio_get_state(address, &state);
  switch (state) {
    case GPIO_STATE_LOW:
      // TODO(ELEC-355): raise charger attached
      break;
    case GPIO_STATE_HIGH:
      // TODO(ELEC-355): raise charger attached
    case NUM_GPIO_STATES:
    default:
      // Unreachable
      break;
  }
}

StatusCode init_charger_pin(const GPIOAddress *address) {
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
