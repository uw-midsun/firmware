#include "lights_gpio.h"
#include "event_queue.h"
#include "gpio.h"
#include "gpio_mcu.h"
#include "lights_config.h"
#include "lights_events.h"
#include "status.h"

static StatusCode prv_search_mappings_table(LightsConfig *conf, const Event *e, uint16_t *bitset) {
  for (uint8_t i = 0; i < conf->num_supported_events; i++) {
    if (e->id == conf->event_mappings[i][0]) {
      *bitset = conf->event_mappings[i][1];
      return STATUS_CODE_OK;
    }
  }
  return STATUS_CODE_INVALID_ARGS;
}

static StatusCode prv_set_peripherals(LightsConfig *conf, uint16_t bitset, GPIOState state) {
  const GPIOAddress *addresses = conf->addresses;
  while (bitset) {
    uint8_t i = __builtin_ffs(bitset) - 1;  // index of first 1 bit
    status_ok_or_return(gpio_set_state(&addresses[i], state));
    bitset &= ~(1 << i);  // bit is read, so we clear it
  }
  return STATUS_CODE_OK;
}

StatusCode lights_gpio_init(LightsConfig *conf) {
  GPIOSettings settings = {
    .direction = GPIO_DIR_OUT,       //
    .state = GPIO_STATE_HIGH,        //
    .resistor = GPIO_RES_NONE,       //
    .alt_function = GPIO_ALTFN_NONE  //
  };

  for (uint8_t i = 0; i < conf->num_addresses; i++) {
    status_ok_or_return(gpio_init_pin(&conf->addresses[i], &settings));
  }
  return STATUS_CODE_OK;
}

StatusCode lights_gpio_process_event(LightsConfig *conf, const Event *e) {
  uint16_t bitset_map;
  status_ok_or_return(prv_search_mappings_table(conf, e, &bitset_map));
  // All the lights are active low, so we negate data field
  return prv_set_peripherals(conf, bitset_map, !e->data);
}
