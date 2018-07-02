#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "delay.h"       
#include "gpio.h"        
#include "interrupt.h"   
#include "misc.h"        
#include "soft_timer.h"  
#include "i2c.h"

#define SOLAR_SENSE_I2C_BUS_SDA \
  { GPIO_PORT_B, 9 }
#define SOLAR_SENSE_I2C_BUS_SCL \
  { GPIO_PORT_B, 8 }

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

  // TODO: 
  // 1. i2c speed? 
  // 2. port? 

  const I2CSettings i2c_settings = {
    .speed = I2C_SPEED_FAST,    //
    .sda = SOLAR_SENSE_I2C_BUS_SDA,  //
    .scl = SOLAR_SENSE_I2C_BUS_SCL,  //
  };

  i2c_init(I2C_PORT_1, &i2c_settings);

  for (size_t i = 0; i < SIZEOF_ARRAY(leds); i++) {
    gpio_init_pin(&leds[i], &led_settings);
  }

  while (true) {
    for (size_t i = 0; i < SIZEOF_ARRAY(leds); i++) {
      gpio_toggle_state(&leds[i]);
      delay_ms(50);
    }
  }

  return 0;
}


