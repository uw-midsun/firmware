#include "event_queue.h"
#include "status.h"
#include "gpio.h"
#include "lights_events.h"
#include "lights_gpio.h"

#define NUM_PERIPH_FRONT 4
#define NUM_PERIPH_REAR 4
#define SOMEPORT 0
#define SOMEPIN 0

static GPIOAddress s_address_lookup[] = {
  [EVENT_HEADLIGHTS] = { .port = 0, .pin = 0 },    //
  [EVENT_SIGNAL_LEFT] = { .port = 0, .pin = 0 },   //
  [EVENT_SIGNAL_RIGHT] = { .port = 0, .pin = 0 },  //
  [EVENT_HORN] = { .port = 0, .pin = 0 },          //
  [EVENT_BRAKES] = { .port = 0, .pin = 0 },        //
  [EVENT_STROBE] = { .port = 0, .pin = 0 }         //
};

// initializes both boards gpio types
StatusCode lights_periph_init(BoardType boardtype) {
  // same settings are used for all ports
  GPIOSettings settings = { .direction = GPIO_DIR_OUT,
                                     .state = GPIO_STATE_LOW,    // defaulted to LOW
                                     .resistor = GPIO_RES_NONE,  // TODO(ELEC-165): figure this out
                                     .alt_function = GPIO_ALTFN_NONE };
  
  // initialize front lights board's peripherals
  if (boardtype == LIGHTS_BOARD_FRONT) {
    s_address_lookup[EVENT_SIGNAL_LEFT] = (GPIOAddress) { .port = 0, .pin = 0 };
    s_address_lookup[EVENT_SIGNAL_RIGHT] = (GPIOAddress) { .port = 0, .pin = 0 };
    gpio_init_pin(&s_address_lookup[EVENT_HEADLIGHTS], &settings);
    gpio_init_pin(&s_address_lookup[EVENT_SIGNAL_LEFT], &settings);
    gpio_init_pin(&s_address_lookup[EVENT_SIGNAL_RIGHT], &settings);
    gpio_init_pin(&s_address_lookup[EVENT_HORN], &settings);
    // init rear board's peripherals
  } else {
    s_address_lookup[EVENT_SIGNAL_LEFT] = (GPIOAddress) { .port = 0, .pin = 0 };
    s_address_lookup[EVENT_SIGNAL_RIGHT] = (GPIOAddress) { .port = 0, .pin = 0 };
    gpio_init_pin(&s_address_lookup[EVENT_BRAKES], &settings);
    gpio_init_pin(&s_address_lookup[EVENT_SIGNAL_LEFT], &settings);
    gpio_init_pin(&s_address_lookup[EVENT_SIGNAL_RIGHT], &settings);
    gpio_init_pin(&s_address_lookup[EVENT_STROBE], &settings);
  }
  return STATUS_CODE_OK;
}

// Takes Event as input, uses the data field
// to determine whether to turn on or turn off
// the peripheral
// used for strobe, headlights, horn and brakes
StatusCode lights_gpio_set(Event e) {
  return gpio_set_state(&s_address_lookup[e.id], 
                          (e.data) ? GPIO_STATE_HIGH : GPIO_STATE_LOW);
}

// figure out whether it's the front board or the back board
// TODO(ELEC-165): figure out which pin to look the boardtype from
StatusCode get_board_type(BoardType* type) {
  const GPIOAddress board_type_address = { .port = SOMEPORT, .pin = SOMEPIN };
  GPIOState state;
  StatusCode state_status = gpio_get_state(&board_type_address, &state);
  if (state_status != STATUS_CODE_OK) {
    return state_status;
  }
  if (state == GPIO_STATE_LOW) {
    *type = LIGHTS_BOARD_FRONT;
  } else if (state == GPIO_STATE_HIGH) {
    *type = LIGHTS_BOARD_REAR;
  }
  return STATUS_CODE_OK;
}
