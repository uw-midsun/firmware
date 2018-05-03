#include "lights_gpio.h"
#include "event_queue.h"
#include "gpio.h"
#include "gpio_mcu.h"
#include "lights_events.h"
#include "lights_gpio_config.h"
#include "status.h"

// Searches the event-to-peripheral-set mapping table for a mapping matching event e.
static StatusCode prv_search_mappings_table(LightsGPIO *lights_gpio, const Event *e, LightsGPIOPeripheralMapping *mapping) {
  for (uint8_t i = 0; i < lights_gpio->num_event_mappings; i++) {
    if (e->id == lights_gpio->event_mappings[i].event_id) {
      *mapping = lights_gpio->event_mappings[i].peripheral_mapping;
      return STATUS_CODE_OK;
    }
  }
  return status_msg(STATUS_CODE_INVALID_ARGS, "Unsupported event");
}

// Sets the state of all of the peripherals in the peripheral mapping
static StatusCode prv_set_peripherals(LightsGPIO *lights_gpio, uint16_t mapping, LightsGPIOState state) {
  // All the lights are active low, so we negate LightsGPIOState.
  GPIOState gpio_state = !((GPIOState) state);
  const GPIOAddress *peripherals = lights_gpio->peripherals;
  while (mapping) {
    uint8_t i = __builtin_ffs(mapping) - 1;  // index of first 1 bit
    status_ok_or_return(gpio_set_state(&peripherals[i], state));
    mapping &= ~(1 << i);  // bit is read, so we clear it
  }
  return STATUS_CODE_OK;
}

// Initializes all the GPIO pins.
StatusCode lights_gpio_init(LightsGPIO *lights_gpio) {
  GPIOSettings settings = {
    .direction = GPIO_DIR_OUT,       //
    .state = GPIO_STATE_HIGH,        //
    .resistor = GPIO_RES_NONE,       //
    .alt_function = GPIO_ALTFN_NONE  //
  };

  for (uint8_t i = 0; i < lights_gpio->num_peripherals; i++) {
    status_ok_or_return(gpio_init_pin(&lights_gpio->peripherals[i], &settings));
  }
  return STATUS_CODE_OK;
}

// Processes events by turning ON/OFF corresponding gpio pins.
StatusCode lights_gpio_process_event(LightsGPIO *lights_gpio, const Event *e) {
  uint16_t bitset_map = 0;
  status_ok_or_return(prv_search_mappings_table(lights_gpio, e, &bitset_map));
  LightsGPIOState state = (LightsGPIOState) e->data;
  return prv_set_peripherals(lights_gpio, bitset_map, state);
}
