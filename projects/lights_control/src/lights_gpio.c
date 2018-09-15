#include "lights_gpio.h"
#include "event_queue.h"
#include "gpio.h"
#include "gpio_mcu.h"
#include "lights_events.h"
#include "lights_gpio_config.h"
#include "log.h"
#include "status.h"

// Searches the event-mappings table for a mapping matching event's peripheral.
static StatusCode prv_search_mappings_table(const LightsGpio *lights_gpio, const Event *e,
                                            LightsGpioOutputBitset *mapping) {
  for (uint8_t i = 0; i < lights_gpio->num_event_mappings; i++) {
    if (e->data == lights_gpio->event_mappings[i].peripheral) {
      *mapping = lights_gpio->event_mappings[i].output_mapping;
      return STATUS_CODE_OK;
    }
  }
  return status_msg(STATUS_CODE_INVALID_ARGS, "Unsupported peripheral.");
}

// Sets the state of all of the outputs in the output mapping.
static StatusCode prv_set_outputs(LightsGpio *lights_gpio, LightsGpioOutputBitset mapping,
                                  LightsGpioState state) {
  while (mapping) {
    uint8_t i = __builtin_ffs(mapping) - 1;  // index of first 1 bit
    LightsGpioOutput output = lights_gpio->outputs[i];
    // Based on the polarity of the output, and the desired state, decide the gpio pin state.
    GpioState gpio_state =
        (output.polarity == LIGHTS_GPIO_POLARITY_ACTIVE_HIGH)
            ? ((state == LIGHTS_GPIO_STATE_ON) ? GPIO_STATE_HIGH : GPIO_STATE_LOW)
            : ((state == LIGHTS_GPIO_STATE_ON) ? GPIO_STATE_LOW : GPIO_STATE_HIGH);
    status_ok_or_return(gpio_set_state(&output.address, gpio_state));
    mapping &= ~(1 << i);  // Bit is read, so we clear it.
  }
  return STATUS_CODE_OK;
}

StatusCode lights_gpio_init(const LightsGpio *lights_gpio) {
  GpioSettings settings = {
    .direction = GPIO_DIR_OUT,       //
    .state = GPIO_STATE_HIGH,        //
    .resistor = GPIO_RES_NONE,       //
    .alt_function = GPIO_ALTFN_NONE  //
  };
  // Initializes gpio pins.
  for (uint8_t i = 0; i < lights_gpio->num_outputs; i++) {
    status_ok_or_return(gpio_init_pin(&(lights_gpio->outputs[i].address), &settings));
  }
  // Makes sure all lights are initialized to be turned off.
  for (uint8_t i = 0; i < lights_gpio->num_outputs; i++) {
    LightsGpioOutput output = lights_gpio->outputs[i];
    GpioState gpio_state =
        (output.polarity == LIGHTS_GPIO_POLARITY_ACTIVE_HIGH) ? GPIO_STATE_LOW : GPIO_STATE_HIGH;

    status_ok_or_return(gpio_set_state(&(lights_gpio->outputs[i].address), gpio_state));
  }
  return STATUS_CODE_OK;
}

StatusCode lights_gpio_process_event(const LightsGpio *lights_gpio, const Event *e) {
  // If event doesn't belong to lights_gpio, it ignores the event.
  if ((e->id != LIGHTS_EVENT_GPIO_OFF) && (e->id != LIGHTS_EVENT_GPIO_ON)) {
    return STATUS_CODE_OK;
  }
  LOG_DEBUG("Turning %s: %d\n", (e->id == LIGHTS_EVENT_GPIO_ON) ? "ON" : "OFF", e->data);
  LightsGpioOutputBitset output_bitset = 0;
  status_ok_or_return(prv_search_mappings_table(lights_gpio, e, &output_bitset));
  LightsGpioState state =
      (e->id == LIGHTS_EVENT_GPIO_OFF) ? LIGHTS_GPIO_STATE_OFF : LIGHTS_GPIO_STATE_ON;
  return prv_set_outputs(lights_gpio, output_bitset, state);
}
