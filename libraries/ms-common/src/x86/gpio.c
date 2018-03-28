#include "gpio.h"

#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>

#include "status.h"
#include "x86_cmd.h"
#include "gpio_it.h"

static GPIOSettings s_pin_settings[GPIO_TOTAL_PINS];
static uint8_t s_gpio_pin_input_value[GPIO_TOTAL_PINS];

static uint32_t prv_get_index(const GPIOAddress *address) {
  return address->port * (uint32_t)NUM_GPIO_PORTS + address->pin;
}

// TCP command handler
// Protocol: [0-3] = command string
//           [4]   = space delimiter
//           [5]   = port
//           [6]   = pin
//           [7]   = state
//           [8]   = null terminator
static void prv_x86_cmd_handler(const char *cmd_str, void *context) {
  GPIOAddress address = { .port = (uint8_t)cmd_str[5], .pin = (uint8_t)cmd_str[6] };
  GPIOState state = (GPIOState)cmd_str[7];

  if (address.port >= NUM_GPIO_PORTS || address.pin >= GPIO_PINS_PER_PORT ||
      state >= NUM_GPIO_STATES) {
    return;
  }

  if (s_pin_settings[prv_get_index(&address)].direction == GPIO_DIR_IN) {
    s_gpio_pin_input_value[prv_get_index(&address)] = state;
    gpio_it_trigger_interrupt(&address);
  }
  return;
}

StatusCode gpio_init(void) {
  GPIOSettings default_settings = {
    .direction = GPIO_DIR_IN,
    .state = GPIO_STATE_LOW,
    .resistor = GPIO_RES_NONE,
    .alt_function = GPIO_ALTFN_NONE,
  };
  for (uint32_t i = 0; i < GPIO_TOTAL_PINS; i++) {
    s_pin_settings[i] = default_settings;
    s_gpio_pin_input_value[i] = 0;
  }

  x86_cmd_register_handler("gpio", prv_x86_cmd_handler, NULL);

  return STATUS_CODE_OK;
}

StatusCode gpio_init_pin(const GPIOAddress *address, const GPIOSettings *settings) {
  if (address->port >= NUM_GPIO_PORTS || address->pin >= GPIO_PINS_PER_PORT ||
      settings->direction >= NUM_GPIO_DIRS || settings->state >= NUM_GPIO_STATES ||
      settings->resistor >= NUM_GPIO_RESES || settings->alt_function >= NUM_GPIO_ALTFNS) {
    return status_code(STATUS_CODE_INVALID_ARGS);
  }

  s_pin_settings[prv_get_index(address)] = *settings;
  return STATUS_CODE_OK;
}

StatusCode gpio_set_pin_state(const GPIOAddress *address, GPIOState state) {
  if (address->port >= NUM_GPIO_PORTS || address->pin >= GPIO_PINS_PER_PORT ||
      state >= NUM_GPIO_STATES) {
    return status_code(STATUS_CODE_INVALID_ARGS);
  }

  s_pin_settings[prv_get_index(address)].state = state;
  return STATUS_CODE_OK;
}

StatusCode gpio_toggle_state(const GPIOAddress *address) {
  if (address->port >= NUM_GPIO_PORTS || address->pin >= GPIO_PINS_PER_PORT) {
    return status_code(STATUS_CODE_INVALID_ARGS);
  }

  uint32_t index = prv_get_index(address);
  if (s_pin_settings[index].state == GPIO_STATE_LOW) {
    s_pin_settings[index].state = GPIO_STATE_HIGH;
  } else {
    s_pin_settings[index].state = GPIO_STATE_LOW;
  }
  return STATUS_CODE_OK;
}

StatusCode gpio_get_value(const GPIOAddress *address, GPIOState *state) {
  if (address->port >= NUM_GPIO_PORTS || address->pin >= GPIO_PINS_PER_PORT) {
    return status_code(STATUS_CODE_INVALID_ARGS);
  }

  uint32_t index = prv_get_index(address);

  // Behave how hardware does when the direction is set to out.
  if (s_pin_settings[index].direction != GPIO_DIR_IN) {
    *state = s_pin_settings[index].state;
  } else {
    *state = s_gpio_pin_input_value[index];
  }
  return STATUS_CODE_OK;
}
