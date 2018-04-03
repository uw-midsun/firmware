#include "lights_gpio.h"
#include "event_queue.h"
#include "gpio.h"
#include "gpio_mcu.h"
#include "lights_config.h"
#include "lights_events.h"
#include "status.h"

static StatusCode prv_set_peripherals(uint16_t bitset, GPIOState state, LightsConfig *conf) {
  const GPIOAddress *addresses = conf->addresses;
  while (bitset) {
    uint8_t i = __builtin_ffs(bitset) - 1;  // index of first 1 bit
    status_ok_or_return(gpio_set_state(&addresses[i], state));
    bitset &= ~(1 << i);  // bit is read, so we clear it
  }
  return STATUS_CODE_OK;
}

StatusCode lights_gpio_init(LightsConfig *conf) {
  for (uint8_t i = 0; i < *conf->num_addresses; i++) {
    status_ok_or_return(gpio_init_pin(&conf->addresses[i], conf->gpio_settings_out));
  }
  return STATUS_CODE_OK;
}

StatusCode lights_gpio_process_event(const Event *e, LightsConfig *conf) {
  if (e->id >= NUM_LIGHTS_EVENTS) {
    return STATUS_CODE_INVALID_ARGS;
  }
  uint16_t *event_mapping = conf->event_mappings;
  uint16_t bitset_map = event_mapping[e->id];
  // All the lights are active low, so we negate data field
  return prv_set_peripherals(bitset_map, !e->data, conf);
}
