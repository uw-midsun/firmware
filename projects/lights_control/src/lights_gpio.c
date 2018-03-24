#include "lights_gpio.h"
#include "event_queue.h"
#include "gpio.h"
#include "gpio_mcu.h"
#include "lights_events.h"
#include "status.h"

#define LIGHTS_HORN_PERIPHERAL 0
#define NUM_LIGHTS_HORN_PERIPHERALS 1
#define LIGHTS_STROBE_PERIPHERAL 0
#define NUM_LIGHTS_STROBE_PERIPHERALS 1

typedef enum {
  LIGHTS_HIGH_BEAMS_RIGHT_PERIPHERAL = 0,
  LIGHTS_HIGH_BEAMS_LEFT_PERIPHERAL,
  NUM_LIGHTS_HIGH_BEAMS_PERIPHERALS
} LightsHighBeamsPeripherals;

typedef enum {
  LIGHTS_LOW_BEAMS_RIGHT_PERIPHERAL = 0,
  LIGHTS_LOW_BEAMS_LEFT_PERIPHERAL,
  NUM_LIGHTS_LOW_BEAMS_PERIPHERALS
} LightsLowBeamsPeripherals;

typedef enum {
  LIGHTS_DRL_RIGHT_PERIPHERAL = 0,
  LIGHTS_DRL_LEFT_PERIPHERAL,
  NUM_LIGHTS_DRL_PERIPHERALS
} LightsDRLPeripherals;

typedef enum {
  LIGHTS_RIGHT_REAR_BRAKE_PERIPHERAL = 0,
  LIGHTS_RIGHT_REAR_OUTER_BRAKE_PERIPHERAL,
  LIGHTS_LEFT_REAR_BRAKE_PERIPHERAL,
  LIGHTS_LEFT_REAR_OUTER_BRAKE_PERIPHERAL,
  LIGHTS_CENTRE_BRAKE_PERIPHERAL,
  NUM_LIGHTS_BRAKES_PERIPHERALS
} LightsBrakesPeripherals;

typedef enum {
  LIGHTS_SIDE_LEFT_INDICATOR_PERIPHERAL = 0,
  LIGHTS_LEFT_TURN_PERIPHERAL,
  NUM_LIGHTS_FRONT_SIGNAL_LEFT_PERIPHERALS
} LightsFrontSignalLeftPeripherals;

typedef enum {
  LIGHTS_SIDE_RIGHT_INDICATOR_PERIPHERAL = 0,
  LIGHTS_RIGHT_TURN_PERIPHERAL,
  NUM_LIGHTS_FRONT_SIGNAL_RIGHT_PERIPHERALS
} LightsFrontSignalRightPeripherals;

typedef enum {
  LIGHTS_LEFT_REAR_OUTER_TURN_PERIPHERAL = 0,
  LIGHTS_LEFT_REAR_TURN_PERIPHERAL,
  NUM_LIGHTS_REAR_SIGNAL_LEFT_PERIPHERALS
} LightsRearSignalLeftPeripherals;

typedef enum {
  LIGHTS_RIGHT_REAR_OUTER_TURN_PERIPHERAL = 0,
  LIGHTS_RIGHT_REAR_TURN_PERIPHERAL,
  NUM_LIGHTS_REAR_SIGNAL_RIGHT_PERIPHERALS
} LightsRearSignalRightPeripherals;

static const GPIOAddress s_horn_peripheral_addresses[NUM_LIGHTS_HORN_PERIPHERALS] = {
  [LIGHTS_HORN_PERIPHERAL] = { .port = GPIO_PORT_B, .pin = 11 },  //
};

static const GPIOAddress s_high_beams_peripheral_addresses[NUM_LIGHTS_HIGH_BEAMS_PERIPHERALS] = {
  [LIGHTS_HIGH_BEAMS_RIGHT_PERIPHERAL] = { .port = GPIO_PORT_B, .pin = 1 },  //
  [LIGHTS_HIGH_BEAMS_LEFT_PERIPHERAL] = { .port = GPIO_PORT_B, .pin = 15 },  //
};

static const GPIOAddress s_low_beams_peripheral_addresses[NUM_LIGHTS_LOW_BEAMS_PERIPHERALS] = {
  [LIGHTS_LOW_BEAMS_RIGHT_PERIPHERAL] = { .port = GPIO_PORT_B, .pin = 2 },  //
  [LIGHTS_LOW_BEAMS_LEFT_PERIPHERAL] = { .port = GPIO_PORT_A, .pin = 8 },   //
};

static const GPIOAddress s_drl_peripheral_addresses[NUM_LIGHTS_DRL_PERIPHERALS] = {
  [LIGHTS_DRL_RIGHT_PERIPHERAL] = { .port = GPIO_PORT_B, .pin = 0 },  //
  [LIGHTS_DRL_LEFT_PERIPHERAL] = { .port = GPIO_PORT_A, .pin = 10 },  //
};

static const GPIOAddress s_brakes_peripheral_addresses[NUM_LIGHTS_BRAKES_PERIPHERALS] = {
  [LIGHTS_RIGHT_REAR_BRAKE_PERIPHERAL] = { .port = GPIO_PORT_B, .pin = 1 },        //
  [LIGHTS_RIGHT_REAR_OUTER_BRAKE_PERIPHERAL] = { .port = GPIO_PORT_B, .pin = 2 },  //
  [LIGHTS_LEFT_REAR_BRAKE_PERIPHERAL] = { .port = GPIO_PORT_B, .pin = 15 },        //
  [LIGHTS_LEFT_REAR_OUTER_BRAKE_PERIPHERAL] = { .port = GPIO_PORT_A, .pin = 8 },   //
  [LIGHTS_CENTRE_BRAKE_PERIPHERAL] = { .port = GPIO_PORT_B, .pin = 14 },           //
};

static const GPIOAddress s_strobe_peripheral_addresses[NUM_LIGHTS_STROBE_PERIPHERALS] = {
  [LIGHTS_STROBE_PERIPHERAL] = { .port = GPIO_PORT_B, .pin = 11 },  //
};

static const GPIOAddress
    s_front_signal_left_peripheral_addresses[NUM_LIGHTS_FRONT_SIGNAL_LEFT_PERIPHERALS] = {
      [LIGHTS_SIDE_LEFT_INDICATOR_PERIPHERAL] = { .port = GPIO_PORT_B, .pin = 14 },  //
      [LIGHTS_LEFT_TURN_PERIPHERAL] = { .port = GPIO_PORT_A, .pin = 9 },             //
    };

static const GPIOAddress
    s_front_signal_right_peripheral_addresses[NUM_LIGHTS_FRONT_SIGNAL_RIGHT_PERIPHERALS] = {
      [LIGHTS_SIDE_RIGHT_INDICATOR_PERIPHERAL] = { .port = GPIO_PORT_B, .pin = 12 },  //
      [LIGHTS_RIGHT_TURN_PERIPHERAL] = { .port = GPIO_PORT_B, .pin = 10 },            //
    };

static const GPIOAddress
    s_rear_signal_left_peripheral_addresses[NUM_LIGHTS_REAR_SIGNAL_LEFT_PERIPHERALS] = {
      [LIGHTS_LEFT_REAR_OUTER_TURN_PERIPHERAL] = { .port = GPIO_PORT_A, .pin = 10 },  //
      [LIGHTS_LEFT_REAR_TURN_PERIPHERAL] = { .port = GPIO_PORT_A, .pin = 9 },         //
    };

static const GPIOAddress
    s_rear_signal_right_peripheral_addresses[NUM_LIGHTS_REAR_SIGNAL_RIGHT_PERIPHERALS] = {
      [LIGHTS_RIGHT_REAR_OUTER_TURN_PERIPHERAL] = { .port = GPIO_PORT_B, .pin = 0 },  //
      [LIGHTS_RIGHT_REAR_TURN_PERIPHERAL] = { .port = GPIO_PORT_B, .pin = 10 },       //
    };

static const GPIOAddress s_board_type_address = { .port = GPIO_PORT_B, .pin = 13 };

static LightsBoard s_board;

static const GPIOAddress *s_address_front_lookup[] = {
  [LIGHTS_EVENT_HORN] = s_horn_peripheral_addresses,                        //
  [LIGHTS_EVENT_HIGH_BEAMS] = s_high_beams_peripheral_addresses,            //
  [LIGHTS_EVENT_LOW_BEAMS] = s_low_beams_peripheral_addresses,              //
  [LIGHTS_EVENT_DRL] = s_drl_peripheral_addresses,                          //
  [LIGHTS_EVENT_SIGNAL_LEFT] = s_front_signal_left_peripheral_addresses,    //
  [LIGHTS_EVENT_SIGNAL_RIGHT] = s_front_signal_right_peripheral_addresses,  //
};

static const GPIOAddress *s_address_rear_lookup[] = {
  [LIGHTS_EVENT_BRAKES] = s_brakes_peripheral_addresses,
  [LIGHTS_EVENT_STROBE] = s_strobe_peripheral_addresses,
  [LIGHTS_EVENT_SIGNAL_LEFT] = s_rear_signal_left_peripheral_addresses,
  [LIGHTS_EVENT_SIGNAL_RIGHT] = s_rear_signal_right_peripheral_addresses,
};

static StatusCode prv_init_peripheral(GPIOAddress *peripherals) {
  uint8_t i;

  // same settings are used for all ports
  GPIOSettings settings = { .direction = GPIO_DIR_OUT,
                            .state = GPIO_STATE_HIGH,
                            .resistor = GPIO_RES_NONE,
                            .alt_function = GPIO_ALTFN_NONE };

  for (i = 0; i < SIZEOF_ARRAY(peripherals); i++) {
    StatusCode status = gpio_init_pin(&peripherals[i], &settings);
    if (status != STATUS_CODE_OK) {
      return status;
    }
  }
  return STATUS_CODE_OK;
}

StatusCode lights_gpio_init() {
  uint8_t i;

  const GPIOAddress **address_lookup;
  address_lookup = (s_board == LIGHTS_BOARD_FRONT) ? s_address_front_lookup : s_address_rear_lookup;

  for (i = 0; i < SIZEOF_ARRAY(address_lookup); i++) {
    StatusCode status = prv_init_peripheral(address_lookup[i]);
    if (status != STATUS_CODE_OK) {
      return status;
    }
  }
  return STATUS_CODE_OK;
}

StatusCode lights_gpio_get_lights_board(LightsBoard *board) {
  GPIOSettings settings = { .direction = GPIO_DIR_OUT,
                            .state = GPIO_STATE_HIGH,
                            .resistor = GPIO_RES_NONE,
                            .alt_function = GPIO_ALTFN_NONE };

  // initializing the board type pin, before reading the board type from it
  StatusCode init_status = gpio_init_pin(&s_board_type_address, &settings);
  if (init_status != STATUS_CODE_OK) {
    return init_status;
  }

  // reading the state to know the board type
  GPIOState state;
  StatusCode state_status = gpio_get_state(&s_board_type_address, &state);
  if (state_status != STATUS_CODE_OK) {
    return state_status;
  }
  if (state == GPIO_STATE_LOW) {
    s_board = LIGHTS_BOARD_FRONT;
    *board = LIGHTS_BOARD_FRONT;
  } else if (state == GPIO_STATE_HIGH) {
    s_board = LIGHTS_BOARD_REAR;
    *board = LIGHTS_BOARD_REAR;
  }
  return STATUS_CODE_OK;
}

StatusCode lights_gpio_set(Event *e) {
  const GPIOAddress **address_lookup;
  uint8_t i;
  address_lookup = (s_board == LIGHTS_BOARD_FRONT) ? s_address_front_lookup : s_address_rear_lookup;
  GPIOAddress *peripheral_addresses = address_lookup[e->id];
  for (i = 0; i < SIZEOF_ARRAY(peripheral_addresses); i++) {
    StatusCode status = gpio_set_state(&peripheral_addresses[i], e->data);
    if (status != STATUS_CODE_OK) {
      return status;
    }
  }
  return STATUS_CODE_OK;
}
