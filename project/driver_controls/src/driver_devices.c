#include <stddef.h>

#include "driver_devices.h"
#include "gpio_it.h"

void driver_controls_init() {
  gpio_init();
  interrupt_init();
  gpio_it_init();
}

void driver_controls_add_device(Device *device) {
  GPIOSettings gpio_settings = { device->direction, GPIO_STATE_LOW,
                                  GPIO_RES_NONE, device->alt_function };

  InterruptSettings it_settings = { INTERRUPT_TYPE_INTERRUPT, INTERRUPT_PRIORITY_NORMAL };

  gpio_init_pin(&device->address, &gpio_settings);

  if (device->callback != NULL) {
    gpio_it_register_interrupt(&device->address, &it_settings, device->edge,
                                device->callback, device->context);
  }
}
