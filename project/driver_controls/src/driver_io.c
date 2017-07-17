#include <stddef.h>

#include "gpio_it.h"
#include "driver_io.h"
#include "driver_callback.h"

// Arbitrary thresholds for gas pedal
#define COAST_THRESHOLD 1000
#define DRIVE_THRESHOLD 3000

typedef struct DriverIOSettings {
  GPIOAddress address;
  GPIODir direction;
  GPIOAltFn alt_function;
  InterruptEdge edge;
  DriverIOCallback callback;
} DriverIOSettings;

// Lookup table to map specific input devices to specific pins
static DriverIOData s_devices[NUM_DRIVER_IO_PIN] = {
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

static DriverIOSettings prv_settings(GPIOAddress address, GPIODir dir, InterruptEdge edge,
                                      GPIOAltFn altfn, void *context) {
  DriverIOSettings settings = { .address = address, .direction = dir, .edge = edge,
    .alt_function = altfn, .callback = context };
  return settings;
}

static void prv_init_pin(DriverIOSettings *driver_io_data) {
  GPIOSettings gpio_settings = { driver_io_data->direction, GPIO_STATE_LOW,
                                  GPIO_RES_NONE, driver_io_data->alt_function };
  InterruptSettings it_settings = { INTERRUPT_TYPE_INTERRUPT, INTERRUPT_PRIORITY_LOW };

  gpio_init_pin(&driver_io_data->address, &gpio_settings);

  if (driver_io_data->callback != NULL) {
    gpio_it_register_interrupt(&driver_io_data->address, &it_settings, driver_io_data->edge,
                                driver_io_data->callback, &s_devices[driver_io_data->address.pin]);
  }
}

void driver_io_init() {
  gpio_init();
  interrupt_init();
  gpio_it_init();

  // Configure driver devices with their individual settings
  DriverIOSettings inputs[] = {
    prv_settings((GPIOAddress){ GPIO_PORT_C, DRIVER_IO_PIN_0 },
      GPIO_DIR_IN, INTERRUPT_EDGE_RISING, GPIO_ALTFN_NONE, driver_callback_input),

    prv_settings((GPIOAddress){ GPIO_PORT_C, DRIVER_IO_PIN_1 },
      GPIO_DIR_IN, INTERRUPT_EDGE_RISING_FALLING, GPIO_ALTFN_ANALOG, NULL),

    prv_settings((GPIOAddress){ GPIO_PORT_B, DRIVER_IO_PIN_2 },
      GPIO_DIR_IN, INTERRUPT_EDGE_RISING_FALLING, GPIO_ALTFN_NONE, driver_callback_input),

    prv_settings((GPIOAddress){ GPIO_PORT_B, DRIVER_IO_PIN_3 },
      GPIO_DIR_IN, INTERRUPT_EDGE_RISING_FALLING, GPIO_ALTFN_NONE, driver_callback_input),

    prv_settings((GPIOAddress){ GPIO_PORT_C, DRIVER_IO_PIN_4 },
      GPIO_DIR_IN, INTERRUPT_EDGE_RISING, GPIO_ALTFN_NONE, driver_callback_input),

    prv_settings((GPIOAddress){ GPIO_PORT_C, DRIVER_IO_PIN_5 },
      GPIO_DIR_IN, INTERRUPT_EDGE_RISING, GPIO_ALTFN_NONE, driver_callback_input),

    prv_settings((GPIOAddress){ GPIO_PORT_C, DRIVER_IO_PIN_6 },
      GPIO_DIR_IN, INTERRUPT_EDGE_RISING, GPIO_ALTFN_NONE, driver_callback_input),

    prv_settings((GPIOAddress){ GPIO_PORT_C, DRIVER_IO_PIN_7 },
      GPIO_DIR_IN, INTERRUPT_EDGE_RISING_FALLING, GPIO_ALTFN_NONE, driver_callback_input),

    prv_settings((GPIOAddress){ GPIO_PORT_C, DRIVER_IO_PIN_8 },
      GPIO_DIR_IN, INTERRUPT_EDGE_RISING_FALLING, GPIO_ALTFN_NONE, driver_callback_input),

    prv_settings((GPIOAddress){ GPIO_PORT_C, DRIVER_IO_PIN_9 },
      GPIO_DIR_IN, INTERRUPT_EDGE_RISING, GPIO_ALTFN_NONE, driver_callback_input),

    prv_settings((GPIOAddress){ GPIO_PORT_C, DRIVER_IO_PIN_10 },
      GPIO_DIR_IN, INTERRUPT_EDGE_RISING_FALLING, GPIO_ALTFN_NONE, driver_callback_input)
  };

  for (uint8_t i = 0; i < NUM_DRIVER_IO_PIN; i++) {
    prv_init_pin(&inputs[i]);
  }

  // Initialize analog inputs
  adc_init(ADC_MODE_CONTINUOUS);
  adc_set_channel(ADC_CHANNEL_11, true);
  adc_register_callback(ADC_CHANNEL_11, driver_callback_pedal, NULL);
}
