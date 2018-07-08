#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "delay.h"
#include "gpio.h"
#include "i2c.h"
#include "interrupt.h"
#include "log.h"
#include "misc.h"
#include "soft_timer.h"

#define SOLAR_SENSE_I2C_BUS_SDA \
  { GPIO_PORT_B, 9 }

#define SOLAR_SENSE_I2C_BUS_SCL \
  { GPIO_PORT_B, 8 }

#define SOLAR_SENSE_ADDR (0x68)
#define SOLAR_SENSE_DATA_LEN 3
// Controller board LEDs
static const GPIOAddress leds[] = {
  { .port = GPIO_PORT_B, .pin = 5 },   //
  { .port = GPIO_PORT_B, .pin = 4 },   //
  { .port = GPIO_PORT_B, .pin = 3 },   //
  { .port = GPIO_PORT_A, .pin = 15 },  //
};

int main(void) {
  // Enable various peripherals
  interrupt_init();
  soft_timer_init();
  gpio_init();

  GPIOSettings led_settings = {
    .direction = GPIO_DIR_OUT,        // The pin needs to output.
    .state = GPIO_STATE_HIGH,         // Start in the "on" state.
    .alt_function = GPIO_ALTFN_NONE,  // No connections to peripherals.
    .resistor = GPIO_RES_NONE,        // No need of a resistor to modify floating logic levels.
  };

  const I2CSettings i2c_settings = {
    .speed = I2C_SPEED_STANDARD,     //
    .sda = SOLAR_SENSE_I2C_BUS_SDA,  //
    .scl = SOLAR_SENSE_I2C_BUS_SCL,  //
  };

  i2c_init(I2C_PORT_1, &i2c_settings);

  for (size_t i = 0; i < SIZEOF_ARRAY(leds); i++) {
    gpio_init_pin(&leds[i], &led_settings);
  }

  uint8_t rx_data[SOLAR_SENSE_DATA_LEN];

  StatusCode s = 0;

  while (true) {
    s = i2c_read(I2C_PORT_1, SOLAR_SENSE_ADDR, rx_data, SOLAR_SENSE_DATA_LEN);
    if (status_ok(s)) {
      uint16_t v = 0;
      v |= (rx_data[0] << 8);
      v |= (rx_data[1]);
      LOG_DEBUG("Voltage: %d\n", v);
    } else {
      LOG_DEBUG("Read Fail\n");
    }
    i2c_read(I2C_PORT_1, SOLAR_SENSE_ADDR, rx_data, 1);
    delay_ms(50);
    for (size_t i = 0; i < SIZEOF_ARRAY(leds); i++) {
      gpio_toggle_state(&leds[i]);
    }
  }
  return 0;
}
