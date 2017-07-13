#include <stddef.h>

#include "driver_device.h"
#include "gpio_it.h"

void driver_device_init() {
  gpio_init();
  interrupt_init();
  gpio_it_init();
}

void driver_device_init_pin(DriverDevice *driver_device) {
  GPIOSettings gpio_settings = { driver_device->direction, GPIO_STATE_LOW,
                                  GPIO_RES_NONE, driver_device->alt_function };
  InterruptSettings it_settings = { INTERRUPT_TYPE_INTERRUPT, INTERRUPT_PRIORITY_LOW };

  gpio_init_pin(&driver_device->address, &gpio_settings);

  if (driver_device->callback != NULL) {
    gpio_it_register_interrupt(&driver_device->address, &it_settings, driver_device->edge,
                                driver_device->callback, driver_device->context);
  }
}
