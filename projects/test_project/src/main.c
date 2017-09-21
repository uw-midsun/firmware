// Test program for STM32F072RB discovery boards - attempts to connect to the onboard MEMS
// gyroscope over SPI. Blinks the blue LED on success or the red LED on fail.
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "delay.h"
#include "gpio.h"
#include "interrupt.h"
#include "misc.h"
#include "soft_timer.h"

int main(void) {
  interrupt_init();
  soft_timer_init();
  gpio_init();

  GPIOSettings led_settings = {
    .direction = GPIO_DIR_OUT,
    .state = GPIO_STATE_HIGH,
  };

  const GPIOAddress leds[] = {
    { .port = GPIO_PORT_B, .pin = 5 },   //
    { .port = GPIO_PORT_B, .pin = 4 },   //
    { .port = GPIO_PORT_B, .pin = 3 },   //
    { .port = GPIO_PORT_A, .pin = 15 },  //
  };
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
