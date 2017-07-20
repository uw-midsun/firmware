#include <stddef.h>

#include "adc.h"
#include "gpio_it.h"
#include "driver_io.h"
#include "status.h"

typedef struct DriverIOSettings {
  GPIOAddress address;
  InterruptEdge edge;
  DriverIOData *data;
} DriverIOSettings;

// Index the objects using their respective pins
static DriverIOData s_input_data[] = {
  [0] = { .id = DRIVER_IO_POWER_SWITCH, .event = INPUT_EVENT_POWER },
  [2] = { .id = DRIVER_IO_DIRECTION_SELECTOR, .event = INPUT_EVENT_DIRECTION_SELECTOR_DRIVE },
  [3] = { .id = DRIVER_IO_DIRECTION_SELECTOR, .event = INPUT_EVENT_DIRECTION_SELECTOR_REVERSE },
  [4] = { .id = DRIVER_IO_CRUISE_CONTROL_INC, .event = INPUT_EVENT_CRUISE_CONTROL_INC },
  [5] = { .id = DRIVER_IO_CRUISE_CONTROL_DEC, .event = INPUT_EVENT_CRUISE_CONTROL_DEC },
  [6] = { .id = DRIVER_IO_TURN_SIGNAL, .event = INPUT_EVENT_TURN_SIGNAL_RIGHT },
  [7] = { .id = DRIVER_IO_CRUISE_CONTROL, .event = INPUT_EVENT_CRUISE_CONTROL },
  [8] = { .id = DRIVER_IO_TURN_SIGNAL, .event = INPUT_EVENT_TURN_SIGNAL_LEFT },
  [9] = { .id = DRIVER_IO_HAZARD_LIGHT, .event = INPUT_EVENT_HAZARD_LIGHT },
  [10] = { .id = DRIVER_IO_MECHANICAL_BRAKE, .event = INPUT_EVENT_MECHANICAL_BRAKE }
};

static void prv_init_pin(DriverIOSettings *settings, GPIOSettings *gpio_settings) {
  InterruptSettings it_settings = { INTERRUPT_TYPE_INTERRUPT, INTERRUPT_PRIORITY_LOW };

  gpio_init_pin(&settings->address, gpio_settings);
  gpio_it_register_interrupt(&settings->address, &it_settings, settings->edge,
                              NULL, settings->data);
}

void driver_io_init() {
  // Configure driver devices with their individual settings
  DriverIOSettings digital_inputs[] = {
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
