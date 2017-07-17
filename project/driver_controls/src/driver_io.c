#include <stddef.h>

#include "driver_io.h"
#include "gpio_it.h"
#include "driver_callback.h"

typedef struct DriverIOSettings {
  GPIOAddress address;
  GPIODir direction;
  GPIOAltFn alt_function;
  InterruptEdge edge;
  DriverIOCallback callback;
} DriverIOSettings;

// Lookup table to map specific input devices to specific pins
static DriverIO s_devices[NUM_DRIVER_IO_PIN] = {
  [DRIVER_IO_PIN_0] = { .id = DRIVER_IO_POWER_SWITCH, .event = INPUT_EVENT_POWER },
  [DRIVER_IO_PIN_2] = { .id = DRIVER_IO_DIRECTION_SELECTOR,
                        .event = INPUT_EVENT_DIRECTION_SELECTOR_DRIVE },
  [DRIVER_IO_PIN_3] = { .id = DRIVER_IO_DIRECTION_SELECTOR,
                        .event = INPUT_EVENT_DIRECTION_SELECTOR_REVERSE },
  [DRIVER_IO_PIN_4] = { .id = DRIVER_IO_CRUISE_CONTROL, .event = INPUT_EVENT_CRUISE_CONTROL },
  [DRIVER_IO_PIN_5] = { .id = DRIVER_IO_CRUISE_CONTROL_INC,
                        .event = INPUT_EVENT_CRUISE_CONTROL_INC },
  [DRIVER_IO_PIN_6] = { .id = DRIVER_IO_CRUISE_CONTROL_DEC,
                        .event = INPUT_EVENT_CRUISE_CONTROL_DEC },
  [DRIVER_IO_PIN_7] = { .id = DRIVER_IO_TURN_SIGNAL, .event = INPUT_EVENT_TURN_SIGNAL_LEFT },
  [DRIVER_IO_PIN_8] = { .id = DRIVER_IO_TURN_SIGNAL, .event = INPUT_EVENT_TURN_SIGNAL_RIGHT },
  [DRIVER_IO_PIN_9] = { .id = DRIVER_IO_HAZARD_LIGHT, .event = INPUT_EVENT_HAZARD_LIGHT },
  [DRIVER_IO_PIN_10] = { .id = DRIVER_IO_MECHANICAL_BRAKE, .event = INPUT_EVENT_MECHANICAL_BRAKE }
};

static void prv_init_pin(DriverIOSettings *driver_io) {
  GPIOSettings gpio_settings = { driver_io->direction, GPIO_STATE_LOW,
                                  GPIO_RES_NONE, driver_io->alt_function };
  InterruptSettings it_settings = { INTERRUPT_TYPE_INTERRUPT, INTERRUPT_PRIORITY_LOW };

  gpio_init_pin(&driver_io->address, &gpio_settings);

  if (driver_io->callback != NULL) {
    gpio_it_register_interrupt(&driver_io->address, &it_settings, driver_io->edge,
                                driver_io->callback, &s_devices[driver_io->address.pin]);
  }
}

void driver_io_init() {
  gpio_init();
  interrupt_init();
  gpio_it_init();

  // Configure driver devices with their individual settings
  DriverIOSettings inputs[] = {
    { .address = { GPIO_PORT_C, DRIVER_IO_PIN_0 }, .direction = GPIO_DIR_IN,
      .edge = INTERRUPT_EDGE_RISING, .alt_function = GPIO_ALTFN_NONE,
      .callback = driver_callback_input
    },

    { .address = { GPIO_PORT_C, DRIVER_IO_PIN_1 }, .direction = GPIO_DIR_IN,
      .edge = INTERRUPT_EDGE_RISING_FALLING, .alt_function = GPIO_ALTFN_ANALOG,
      .callback = NULL
    },

    { .address = { GPIO_PORT_B, DRIVER_IO_PIN_2 }, .direction = GPIO_DIR_IN,
      .edge = INTERRUPT_EDGE_RISING_FALLING, .alt_function = GPIO_ALTFN_NONE,
      .callback = driver_callback_input
    },

    { .address = { GPIO_PORT_B, DRIVER_IO_PIN_3 }, .direction = GPIO_DIR_IN,
      .edge = INTERRUPT_EDGE_RISING_FALLING, .alt_function = GPIO_ALTFN_NONE,
      .callback = driver_callback_input
    },

    { .address = { GPIO_PORT_C, DRIVER_IO_PIN_4 }, .direction = GPIO_DIR_IN,
      .edge = INTERRUPT_EDGE_RISING, .alt_function = GPIO_ALTFN_NONE,
      .callback = driver_callback_input
    },

    { .address = { GPIO_PORT_C, DRIVER_IO_PIN_5 }, .direction = GPIO_DIR_IN,
      .edge = INTERRUPT_EDGE_RISING, .alt_function = GPIO_ALTFN_NONE,
      .callback = driver_callback_input
    },

    { .address = { GPIO_PORT_C, DRIVER_IO_PIN_6 }, .direction = GPIO_DIR_IN,
      .edge = INTERRUPT_EDGE_RISING, .alt_function = GPIO_ALTFN_NONE,
      .callback = driver_callback_input
    },

    { .address = { GPIO_PORT_C, DRIVER_IO_PIN_7 }, .direction = GPIO_DIR_IN,
      .edge = INTERRUPT_EDGE_RISING_FALLING, .alt_function = GPIO_ALTFN_NONE,
      .callback = driver_callback_input
    },

    { .address = { GPIO_PORT_C, DRIVER_IO_PIN_8 }, .direction = GPIO_DIR_IN,
      .edge = INTERRUPT_EDGE_RISING_FALLING, .alt_function = GPIO_ALTFN_NONE,
      .callback = driver_callback_input
    },

    { .address = { GPIO_PORT_C, DRIVER_IO_PIN_9 }, .direction = GPIO_DIR_IN,
      .edge = INTERRUPT_EDGE_RISING, .alt_function = GPIO_ALTFN_NONE,
      .callback = driver_callback_input
    },

    { .address = { GPIO_PORT_C, DRIVER_IO_PIN_10 }, .direction = GPIO_DIR_IN,
      .edge = INTERRUPT_EDGE_RISING_FALLING, .alt_function = GPIO_ALTFN_NONE,
      .callback = driver_callback_input
    }
  };

  DriverIOSettings outputs[] = {
    { .address = { GPIO_PORT_C, DRIVER_IO_PIN_11 }, .direction = GPIO_DIR_OUT,
      .alt_function = GPIO_ALTFN_NONE, .callback = NULL }
  };

  for (uint8_t i = 0; i < sizeof(inputs)/sizeof(*inputs); i++) {
    prv_init_pin(&inputs[i]);
  }

  for (uint8_t i = 0; i < sizeof(outputs)/sizeof(*outputs); i++) {
    prv_init_pin(&outputs[i]);
  }

  // Initialize analog inputs
  adc_init(ADC_MODE_CONTINUOUS);
  adc_set_channel(ADC_CHANNEL_11, true);
  adc_register_callback(ADC_CHANNEL_11, driver_callback_pedal, NULL);
}
