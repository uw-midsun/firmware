#include "structs.h"
#include "lights_events.h"
#include "event_queue.h"
#include "gpio.h"

#define NUM_PERIPH_FRONT 4
#define NUM_PERIPH_REAR 4

static GPIOAddress s_front_address_lookup[] = {
  [EVENT_HEADLIGHTS] = { .port = 0, .pin = 0 }, //
  [EVENT_SIGNAL_LEFT] = { .port = 0, .pin = 0 }, //
  [EVENT_SIGNAL_RIGHT] = { .port = 0, .pin = 0 }, //
  [EVENT_HORN] = { .port = 0, .pin = 0 } //
};

static GPIOAddress s_rear_address_lookup[] = {
  [EVENT_BRAKES] = { .port = 0, .pin = 0 }, //
  [EVENT_SIGNAL_LEFT] = { .port = 0, .pin = 0 }, //
  [EVENT_SIGNAL_RIGHT] = { .port = 0, .pin = 0 }, //
  [EVENT_STROBE] = { .port = 0, .pin = 0 } //
};



// same settings are used for all ports
static GPIOSettings s_settings = {
  .direction = GPIO_DIR_OUT,
  .state = GPIO_STATE_LOW, // defaulted to LOW
  .resistor = GPIO_RES_NONE, // TODO: figure this out
  .alt_function = GPIO_ALTFN_NONE
};

BoardType s_boardtype;

StatusCode initialize_peripherals(BoardType boardtype) {
  s_boardtype = boardtype;
  // initialize front lights board's peripherals
  if (boardtype == LIGHTS_BOARD_FRONT) {
    for (uint8_t i = 0; i < NUM_PERIPH_FRONT; i++) {
      gpio_init_pin((s_front_address_lookup + i), &s_settings);
    }
    // init rear board's peripherals
  } else if (boardtype == LIGHTS_BOARD_REAR) {
    for (uint8_t i = 0; i < NUM_PERIPH_REAR; i++) {
      gpio_init_pin((s_rear_address_lookup + i), &s_settings);
    }
  }
  return STATUS_CODE_OK;
}

// Takes Event as input, uses the data field
// to determine whether to turn on or turn off
// the peripheral
// used for strobe, headlights, horn and brakes
StatusCode simple_peripheral(Event e) {
  if (s_boardtype == LIGHTS_BOARD_FRONT) {
    return gpio_set_state(&s_front_address_lookup[e.id], (e.data) ? GPIO_STATE_HIGH : GPIO_STATE_LOW ); \
  } else if (s_boardtype == LIGHTS_BOARD_REAR) {
    return gpio_set_state(&s_rear_address_lookup[e.id], (e.data) ? GPIO_STATE_HIGH : GPIO_STATE_LOW ); \
  }
  return STATUS_CODE_INVALID_ARGS;
}

