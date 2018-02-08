#include "structs.h"
#include "gpio_addresses.h"

// front board peripheral definitions
GPIOAddress ADDRESS_SIGNAL_LEFT_FRONT = {
  .pin = 0,
  .port = 0
};

GPIOAddress ADDRESS_SIGNAL_RIGHT_FRONT = {
  .pin = 0,
  .port = 0
};

GPIOAddress ADDRESS_HORN = {
  .pin = 0,
  .port = 0
};

GPIOAddress ADDRESS_HEADLIGHTS = {
  .pin = 0,
  .port = 0
};

// rear board peripheral definitions

GPIOAddress ADDRESS_SIGNAL_RIGHT_REAR = {
  .pin = 0,
  .port = 0
};

GPIOAddress ADDRESS_SIGNAL_LEFT_REAR = {
  .pin = 0,
  .port = 0
};

GPIOAddress ADDRESS_BRAKE = {
  .pin = 0,
  .port = 0
};

GPIOAddress ADDRESS_STROBE = {
  .pin = 0,
  .port = 0
};

// same settings are used for all ports
static GPIOSettings settings = {
  .direction = GPIO_DIR_OUT,
  .state = GPIO_STATE_LOW, // defaulted to LOW
  .resistor = GPIO_RES_NONE, // TODO: figure this out
  .alt_function = GPIO_ALTFN_NONE
};

StatusCode initialize_peripherals(BoardType boardtype) {
  // initialize front lights board's peripherals
  if (boardtype == LIGHTS_BOARD_FRONT) {
    gpio_init_pin(&ADDRESS_SIGNAL_LEFT_FRONT, &settings);
    gpio_init_pin(&ADDRESS_SIGNAL_RIGHT_FRONT, &settings);
    gpio_init_pin(&ADDRESS_HORN, &settings);
    gpio_init_pin(&ADDRESS_HEADLIGHTS, &settings);
    // init rear board's peripherals
  } else if (boardtype == LIGHTS_BOARD_REAR) {
    gpio_init_pin(&ADDRESS_SIGNAL_LEFT_REAR, &settings);
    gpio_init_pin(&ADDRESS_SIGNAL_RIGHT_REAR, &settings);
    gpio_init_pin(&ADDRESS_BRAKE, &settings);
    gpio_init_pin(&ADDRESS_STROBE, &settings);
  }
  return STATUS_CODE_OK;
}

