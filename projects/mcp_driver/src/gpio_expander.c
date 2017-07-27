#include "gpio_expander.h"
#include "i2c.h"
#include "gpio_it.h"

#include <stdbool.h>

// MCP23008 I2C address 
#define GPIO_EXPANDER_ADDRESS   0x20

// MCP23008 register addresses
#define GPIO_EXPANDER_IODIR     0x0
#define GPIO_EXPANDER_IPOL      0x1
#define GPIO_EXPANDER_GPINTEN   0x2
#define GPIO_EXPANDER_DEFVAL    0x3
#define GPIO_EXPANDER_INTCON    0x4
#define GPIO_EXPANDER_IOCON     0x5
#define GPIO_EXPANDER_GPPU      0x6
#define GPIO_EXPANDER_INTF      0x7
#define GPIO_EXPANDER_INTCAP    0x8
#define GPIO_EXPANDER_GPIO      0x9
#define GPIO_EXPANDER_OLAT      0xA

#define NUM_GPIO_EXPANDER_REGISTERS 11

typedef struct GPIOExpanderInterrupt {
  GPIOExpanderPin pin;
  GPIOExpanderCallback callback;
  void *context;
} GPIOExpanderInterrupt;

static GPIOAddress s_address;
static GPIOExpanderInterrupt s_interrupts[NUM_GPIO_EXPANDER_PIN];
static uint8_t s_register[NUM_GPIO_EXPANDER_REGISTERS];

static void prv_interrupt_handler(GPIOAddress *address, void *context) {

}

static void prv_set_bit(uint8_t *data, GPIOExpanderPin pin, bool bit) {
  printf("%#x -> ", *data);
  if (bit) {
    *data |= (1 << pin);
  } else {
    *data &= !(1 << pin);
  }
  printf("%#x\n", *data);
}

StatusCode gpio_expander_init(GPIOAddress address) {
  s_address = address;

  // Configure the pin to receive interrupts from the MCP23008
  GPIOSettings gpio_settings = { .direction = GPIO_DIR_IN, .alt_function = GPIO_ALTFN_NONE };
  InterruptSettings it_settings = { INTERRUPT_TYPE_INTERRUPT, INTERRUPT_PRIORITY_NORMAL };

  gpio_init_pin(&address, &gpio_settings);
  gpio_it_register_interrupt(&address, &it_settings, INTERRUPT_EDGE_RISING_FALLING,
                              prv_interrupt_handler, NULL);

  // Configure the I2C lines
  I2CSettings settings = {
    .speed = I2C_SPEED_STANDARD,
    .sda = { GPIO_PORT_B, 9 },
    .scl = { GPIO_PORT_B, 8 }
  };

  i2c_init(I2C_PORT_1, &settings);

  for (uint8_t i = 0; i < NUM_GPIO_EXPANDER_REGISTERS; i++) {
    i2c_read_reg(I2C_PORT_1, GPIO_EXPANDER_ADDRESS, i, &s_register[i], 1);
    printf("s_register[%d] = %#x\n", i, s_register[i]);
  }
  // TODO: Reset registers to their default values (Figure out how to use the POR)
}

/*
typedef struct GPIOSettings {
  GPIODir direction;
  GPIOState state;
  GPIORes resistor;
} GPIOSettings;
*/
StatusCode gpio_expander_init_pin(GPIOExpanderPin pin, GPIOSettings *settings) {
  prv_set_bit(&s_register[GPIO_EXPANDER_IODIR], pin, (settings->direction == GPIO_DIR_IN));
  i2c_write_reg(I2C_PORT_1, GPIO_EXPANDER_ADDRESS, GPIO_EXPANDER_IODIR,
                &s_register[GPIO_EXPANDER_IODIR], 1);

  if (settings->direction == GPIO_DIR_OUT) {
    prv_set_bit(&s_register[GPIO_EXPANDER_GPIO], pin, (settings->state == GPIO_STATE_HIGH));
    i2c_write_reg(I2C_PORT_1, GPIO_EXPANDER_ADDRESS, GPIO_EXPANDER_GPIO,
                  &s_register[GPIO_EXPANDER_GPIO], 1);
  }
}