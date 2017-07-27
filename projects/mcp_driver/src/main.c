#include <stdio.h>
#include "i2c.h"
#include "gpio_it.h"
#include "gpio_expander.h"

#define IODIR     0x0
#define IPOL      0x1
#define GPINTEN   0x2
#define DEFVAL    0x3
#define INTCON    0x4
#define IOCON     0x5
#define GPPU      0x6
#define INTF      0x7
#define INTCAP    0x8
#define GPIO      0x9
#define OLAT      0xA

void callback(GPIOAddress *address, void *context) {}

int main() {
  // GPIO initialization
  gpio_init();
  interrupt_init();
  gpio_it_init();

  GPIOAddress address = { GPIO_PORT_A, 0 };
  GPIOSettings settings = { .direction = GPIO_DIR_OUT, .state = GPIO_STATE_HIGH };

  gpio_expander_init(address);

  // Device ID
  I2CAddress addr = 0x20;

  uint8_t io_dir_data = 0x0;
  uint8_t gpio_data = 0x1;
  uint8_t reg_data = 0x0;


  gpio_expander_init_pin(GPIO_EXPANDER_PIN_0, &settings);
  
  while(1) {

  }
}