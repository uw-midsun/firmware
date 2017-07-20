#include <stddef.h>

#include "gpio_it.h"
#include "digital_io.h"
#include "input_event.h"

// Digital device identifiers
typedef enum {
  DIGITAL_IO_POWER_SWITCH = 0,
  DIGITAL_IO_DIRECTION_SELECTOR,
  DIGITAL_IO_CRUISE_CONTROL,
  DIGITAL_IO_CRUISE_CONTROL_INC,
  DIGITAL_IO_CRUISE_CONTROL_DEC,
  DIGITAL_IO_TURN_SIGNAL,
  DIGITAL_IO_HAZARD_LIGHT,
  DIGITAL_IO_MECHANICAL_BRAKE,
  NUM_DIGITAL_IO_INPUTS
} DigitalIODevices;

// Store the id of the device as well as the id of the event the device raises
typedef struct DigitalIOData {
  DigitalIODevices id;
  InputEvent event;
} DigitalIOData;

// Configuration settings for the digital pins
typedef struct DigitalIOSettings {
  GPIOAddress address;
  InterruptEdge edge;
  DigitalIOData *data;
} DigitalIOSettings;

// Index the objects using their respective pins 
static DigitalIOData s_input_data[] = {
  [0] = { .id = DIGITAL_IO_POWER_SWITCH, .event = INPUT_EVENT_POWER },
  [2] = { .id = DIGITAL_IO_DIRECTION_SELECTOR, .event = INPUT_EVENT_DIRECTION_SELECTOR_DRIVE },
  [3] = { .id = DIGITAL_IO_DIRECTION_SELECTOR, .event = INPUT_EVENT_DIRECTION_SELECTOR_REVERSE },
  [4] = { .id = DIGITAL_IO_CRUISE_CONTROL, .event = INPUT_EVENT_CRUISE_CONTROL },
  [5] = { .id = DIGITAL_IO_CRUISE_CONTROL_INC, .event = INPUT_EVENT_CRUISE_CONTROL_INC },
  [6] = { .id = DIGITAL_IO_CRUISE_CONTROL_DEC, .event = INPUT_EVENT_CRUISE_CONTROL_DEC },
  [7] = { .id = DIGITAL_IO_TURN_SIGNAL, .event = INPUT_EVENT_TURN_SIGNAL_RIGHT },
  [8] = { .id = DIGITAL_IO_TURN_SIGNAL, .event = INPUT_EVENT_TURN_SIGNAL_LEFT },
  [9] = { .id = DIGITAL_IO_HAZARD_LIGHT, .event = INPUT_EVENT_HAZARD_LIGHT },
  [10] = { .id = DIGITAL_IO_MECHANICAL_BRAKE, .event = INPUT_EVENT_MECHANICAL_BRAKE }
};

static void prv_get_event(DigitalIOData *digital_io_data, Event *e, GPIOState state) {
  switch (digital_io_data->id) {
    case DIGITAL_IO_DIRECTION_SELECTOR:
      if (state == GPIO_STATE_LOW) {
        e->id = INPUT_EVENT_DIRECTION_SELECTOR_NEUTRAL;
        return;
      }
    case DIGITAL_IO_TURN_SIGNAL:
      if (state == GPIO_STATE_LOW) {
        e->id = INPUT_EVENT_TURN_SIGNAL_NONE;
        return;
      }
    case DIGITAL_IO_POWER_SWITCH:
    case DIGITAL_IO_CRUISE_CONTROL:
    case DIGITAL_IO_CRUISE_CONTROL_INC:
    case DIGITAL_IO_CRUISE_CONTROL_DEC:
    case DIGITAL_IO_HAZARD_LIGHT:
    case DIGITAL_IO_MECHANICAL_BRAKE:
      e->id = digital_io_data->event;
      return;
  }
}

static void prv_input_callback(GPIOAddress *address, void *context) {
  GPIOState state;
  Event e;

  DigitalIOData *data = context;

  gpio_get_value(address, &state);
  prv_get_event(data, &e, state);
  event_raise(e.id, e.data);
}

static void prv_init_pin(DigitalIOSettings *settings, GPIOSettings *gpio_settings) {
  InterruptSettings it_settings = { INTERRUPT_TYPE_INTERRUPT, INTERRUPT_PRIORITY_LOW };

  gpio_init_pin(&settings->address, gpio_settings);
  gpio_it_register_interrupt(&settings->address, &it_settings, settings->edge,
                              prv_input_callback, settings->data);
}

void digital_io_init() {
  // Configure driver devices with their individual settings
  DigitalIOSettings digital_inputs[] = {
    { .address = { GPIO_PORT_C, 0 }, .edge = INTERRUPT_EDGE_RISING,
      .data = &s_input_data[0] },
    { .address = { GPIO_PORT_B, 2 }, .edge = INTERRUPT_EDGE_RISING_FALLING,
      .data = &s_input_data[2] },
    { .address = { GPIO_PORT_B, 3 }, .edge = INTERRUPT_EDGE_RISING_FALLING,
      .data = &s_input_data[3] },
    { .address = { GPIO_PORT_C, 4 }, .edge = INTERRUPT_EDGE_RISING,
      .data = &s_input_data[4] },
    { .address = { GPIO_PORT_C, 5 }, .edge = INTERRUPT_EDGE_RISING,
      .data = &s_input_data[5] },
    { .address = { GPIO_PORT_C, 6 }, .edge = INTERRUPT_EDGE_RISING,
      .data = &s_input_data[6] },
    { .address = { GPIO_PORT_C, 7 }, .edge = INTERRUPT_EDGE_RISING_FALLING,
      .data = &s_input_data[7] },
    { .address = { GPIO_PORT_C, 8 }, .edge = INTERRUPT_EDGE_RISING_FALLING,
      .data = &s_input_data[8] },
    { .address = { GPIO_PORT_C, 9 }, .edge = INTERRUPT_EDGE_RISING,
      .data = &s_input_data[9] },
    { .address = { GPIO_PORT_C, 10 }, .edge = INTERRUPT_EDGE_RISING_FALLING,
      .data = &s_input_data[10] },
  };

  GPIOSettings settings = { GPIO_DIR_IN, GPIO_STATE_LOW, GPIO_RES_NONE, GPIO_ALTFN_NONE };

  for (uint8_t i = 0; i < SIZEOF_ARRAY(digital_inputs); i++) {
    prv_init_pin(&digital_inputs[i], &settings);
  }
}
