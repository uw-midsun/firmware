#include "driver_devices.h"
#include "gpio_it.h"
#include "input_interrupt.h"

typedef struct DeviceSettings {
  GPIOAddress address;
  GPIODir direction;
  InterruptEdge edge;
  GPIOAltFn alt_function;
} DeviceSettings;

void device_init(Devices* devices) {
  gpio_init();
  interrupt_init();
  gpio_it_init();

  // Settings for device initialization
  GPIOSettings gpio_settings = { GPIO_DIR_IN, GPIO_STATE_LOW, GPIO_RES_NONE, GPIO_ALTFN_NONE };
  InterruptSettings it_settings = { INTERRUPT_TYPE_INTERRUPT, INTERRUPT_PRIORITY_NORMAL };

  devices->num_inputs = INPUT_DEVICES;
  devices->num_outputs = OUTPUT_DEVICES;

  memset(devices->inputs, 0, sizeof(GPIOAddress)*INPUT_DEVICES);
  memset(devices->outputs, 0, sizeof(GPIOAddress)*OUTPUT_DEVICES);   

  DeviceSettings input_settings[INPUT_DEVICES] = {
    { { GPIO_PORT_C, 0 }, GPIO_DIR_IN, INTERRUPT_EDGE_RISING, GPIO_ALTFN_NONE },
    { { GPIO_PORT_C, 1 }, GPIO_DIR_IN, INTERRUPT_EDGE_RISING_FALLING, GPIO_ALTFN_ANALOG },
    { { GPIO_PORT_B, 2 }, GPIO_DIR_IN, INTERRUPT_EDGE_RISING_FALLING, GPIO_ALTFN_NONE },
    { { GPIO_PORT_B, 3 }, GPIO_DIR_IN, INTERRUPT_EDGE_RISING_FALLING, GPIO_ALTFN_NONE },
    { { GPIO_PORT_C, 4 }, GPIO_DIR_IN, INTERRUPT_EDGE_RISING, GPIO_ALTFN_NONE },
    { { GPIO_PORT_C, 5 }, GPIO_DIR_IN, INTERRUPT_EDGE_RISING, GPIO_ALTFN_NONE },
    { { GPIO_PORT_C, 6 }, GPIO_DIR_IN, INTERRUPT_EDGE_RISING, GPIO_ALTFN_NONE },
    { { GPIO_PORT_C, 7 }, GPIO_DIR_IN, INTERRUPT_EDGE_RISING_FALLING, GPIO_ALTFN_NONE },
    { { GPIO_PORT_C, 8 }, GPIO_DIR_IN, INTERRUPT_EDGE_RISING_FALLING, GPIO_ALTFN_NONE },
    { { GPIO_PORT_C, 9 }, GPIO_DIR_IN, INTERRUPT_EDGE_RISING, GPIO_ALTFN_NONE },
    { { GPIO_PORT_C, 10 }, GPIO_DIR_IN, INTERRUPT_EDGE_RISING, GPIO_ALTFN_NONE }
  };

  DeviceSettings output_settings[OUTPUT_DEVICES] = {
    { { GPIO_PORT_C, 11 }, GPIO_DIR_OUT, 0, GPIO_ALTFN_NONE } 
  };

  for (uint8_t i=0; i < INPUT_DEVICES; i++) {
    gpio_settings.direction = input_settings[i].direction;
    gpio_settings.alt_function = input_settings[i].alt_function;
    gpio_init_pin(&input_settings[i].address, &gpio_settings);
    gpio_it_register_interrupt(&input_settings[i].address,
            &it_settings,
            input_settings[i].edge,
            input_callback,
            0);
  }

}