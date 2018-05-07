#include "lights_gpio.h"
#include "event_queue.h"
#include "gpio.h"
#include "gpio_mcu.h"
#include "lights_events.h"
#include "lights_gpio_config.h"
#include "status.h"

// Searches the event-to-peripheral-set mapping table for a mapping matching event e.
static StatusCode prv_search_mappings_table(const LightsGPIO *lights_gpio, const Event *e,
                                            LightsGPIOPeripheralMapping *mapping) {
  for (uint8_t i = 0; i < lights_gpio->num_event_mappings; i++) {
    if (e->id == lights_gpio->event_mappings[i].event_id) {
      *mapping = lights_gpio->event_mappings[i].peripheral_mapping;
      return STATUS_CODE_OK;
    }
  }
  return status_msg(STATUS_CODE_INVALID_ARGS, "Unsupported event");
}

// Sets the state of all of the peripherals in the peripheral mapping
static StatusCode prv_set_peripherals(LightsGPIO *lights_gpio, uint16_t mapping,
                                      LightsGPIOState state) {
  while (mapping) {
    uint8_t i = __builtin_ffs(mapping) - 1;  // index of first 1 bit
    LightsGPIOPeripheral peripheral = lights_gpio->peripherals[i];
    // Based on the polarity of the peripheral, and the desired state, decide the gpio pin state.
    GPIOState gpio_state =
        (peripheral.polarity == LIGHTS_GPIO_POLARITY_ACTIVE_HIGH)
            ? ((state == LIGHTS_GPIO_STATE_ON) ? GPIO_STATE_HIGH : GPIO_STATE_LOW)
            : ((state == LIGHTS_GPIO_STATE_ON) ? GPIO_STATE_LOW : GPIO_STATE_HIGH);
    status_ok_or_return(gpio_set_state(&peripheral.address, gpio_state));
    mapping &= ~(1 << i);  // Bit is read, so we clear it
  }
  return STATUS_CODE_OK;
}

// Initializes all the GPIO pins.
StatusCode lights_gpio_init(const LightsGPIO *lights_gpio) {
  GPIOSettings settings = {
    .direction = GPIO_DIR_OUT,       //
    .state = GPIO_STATE_HIGH,        //
    .resistor = GPIO_RES_NONE,       //
    .alt_function = GPIO_ALTFN_NONE  //
  };

  for (uint8_t i = 0; i < lights_gpio->num_peripherals; i++) {
    status_ok_or_return(gpio_init_pin(&(lights_gpio->peripherals[i].address), &settings));
  }
  return STATUS_CODE_OK;
}

// Processes events by turning ON/OFF corresponding gpio pins.
StatusCode lights_gpio_process_event(const LightsGPIO *lights_gpio, const Event *e) {
  if (e->data >= NUM_LIGHTS_GPIO_STATES) {
    return status_code(STATUS_CODE_INVALID_ARGS);
  }
  uint16_t bitset_map = 0;
  status_ok_or_return(prv_search_mappings_table(lights_gpio, e, &bitset_map));
  LightsGPIOState state = (LightsGPIOState)e->data;
  return prv_set_peripherals(lights_gpio, bitset_map, state);
}
