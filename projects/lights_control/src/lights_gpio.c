#include "lights_gpio.h"
#include "event_queue.h"
#include "gpio.h"
#include "gpio_mcu.h"
#include "lights_config.h"
#include "lights_events.h"
#include "status.h"

static const GPIOAddress *s_addresses;

// set to an invalid value, gets set when module gets initialized
static LightsBoard s_board = NUM_LIGHTS_BOARDS;

// mapping events to their corresponding peripherals using bitsets , index of bit corresponds to
// the index of GPIO addresses in lights_config.c
static uint16_t s_front_event_mappings[NUM_FRONT_LIGHTS_EVENTS] = {
  [FRONT_LIGHTS_EVENT_HORN] = 0x001,           //
  [FRONT_LIGHTS_EVENT_HIGH_BEAMS] = 0x006,     //
  [FRONT_LIGHTS_EVENT_LOW_BEAMS] = 0x018,      //
  [FRONT_LIGHTS_EVENT_DRL] = 0x600,            //
  [FRONT_LIGHTS_EVENT_SIGNAL_LEFT] = 0x180,    //
  [FRONT_LIGHTS_EVENT_SIGNAL_RIGHT] = 0x600,   //
  [FRONT_LIGHTS_EVENT_SIGNAL_HAZARD] = 0x780,  //
};

static uint16_t s_rear_event_mappings[NUM_REAR_LIGHTS_EVENTS] = {
  [REAR_LIGHTS_EVENT_STROBE] = 0x001,         //
  [REAR_LIGHTS_EVENT_BRAKES] = 0x03e,         //
  [REAR_LIGHTS_EVENT_SIGNAL_LEFT] = 0x0c0,    //
  [REAR_LIGHTS_EVENT_SIGNAL_RIGHT] = 0x300,   //
  [REAR_LIGHTS_EVENT_SIGNAL_HAZARD] = 0x3c0,  //
};

StatusCode prv_set_peripherals(uint16_t bitset, GPIOState state) {
  uint8_t i;
  while (bitset) {
    i = __builtin_ffs(bitset) - 1;  // index of first 1 bit
    status_ok_or_return(gpio_set_state(&s_addresses[i], state));
    bitset &= ~(1 << i);  // bit is read, so we clear it
  }
  return STATUS_CODE_OK;
}

static StatusCode prv_get_board_type(LightsBoard *board, LightsConfig *config) {
  // initializing board type pin
  status_ok_or_return(gpio_init_pin(config->board_type_address, config->gpio_settings_in));
  // reading the state to know the board type
  GPIOState state;
  StatusCode state_status = gpio_get_state(config->board_type_address, &state);
  status_ok_or_return(state_status);
  *board = (state) ? LIGHTS_BOARD_FRONT : LIGHTS_BOARD_REAR;
  return STATUS_CODE_OK;
}

StatusCode lights_gpio_init() {
  LightsConfig *config = lights_config_load();
  // test modes are used for unit tests
  status_ok_or_return(prv_get_board_type(&s_board, config));
  s_addresses = (s_board) ? config->addresses_rear : config->addresses_front;

  uint8_t num_addresses = (s_board) ? config->num_addresses_rear : config->num_addresses_front;

  for (uint8_t i = 0; i < num_addresses; i++) {
    status_ok_or_return(gpio_init_pin(&s_addresses[i], config->gpio_settings_out));
  }
  return STATUS_CODE_OK;
}

StatusCode lights_gpio_get_lights_board(LightsBoard *board) {
  if (s_board == NUM_LIGHTS_BOARDS) {
    // lights_gpio_init() must be called first
    return STATUS_CODE_UNINITIALIZED;
  }
  *board = s_board;
  return STATUS_CODE_OK;
}

StatusCode lights_gpio_set(const Event *e) {
  if (s_board == NUM_LIGHTS_BOARDS) {
    // lights_gpio_init() must be called first
    return STATUS_CODE_UNINITIALIZED;
  }
  uint8_t num_events = (s_board) ? NUM_REAR_LIGHTS_EVENTS : NUM_FRONT_LIGHTS_EVENTS;
  if (e->id >= num_events) {
    return STATUS_CODE_INVALID_ARGS;
  }
  uint16_t *event_mapping = (s_board) ? s_rear_event_mappings : s_front_event_mappings;
  uint16_t bitset_map = event_mapping[e->id];
  // All the lights are active low, so we negate data field
  return prv_set_peripherals(bitset_map, !e->data);
}

uint16_t *test_lights_gpio_event_mappings(LightsBoard board) {
  return (board) ? s_rear_event_mappings : s_front_event_mappings;
}
