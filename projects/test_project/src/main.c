// Example program for STM32F072 Controller board or Discovery Board.
// Blinks the LEDs sequentially.
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "delay.h"       // For real-time delays
#include "gpio.h"        // General Purpose I/O control.
#include "interrupt.h"   // For enabling interrupts.
#include "misc.h"        // Various helper functions/macros.
#include "soft_timer.h"  // Software timers for scheduling future events.

#include "i2c.h"

static const I2CSettings i2c_settings = {
  .speed = I2C_SPEED_STANDARD, 
  .sda = {.port = GPIO_PORT_B, .pin = 9},
  .scl = {.port = GPIO_PORT_B, .pin = 8},
}; 

// Controller board LEDs
static const GpioAddress leds[] = {
  { .port = GPIO_PORT_B, .pin = 5 },   //
  { .port = GPIO_PORT_B, .pin = 4 },   //
  { .port = GPIO_PORT_B, .pin = 3 },   //
  { .port = GPIO_PORT_A, .pin = 15 },  //
};

// SPI 
const SpiSettings mems_spi_settings = {
  .mode = SPI_MODE_0, 
  .mosi = {.port = GPIO_PORT_B, .pin = 15},
  .miso = {.port = GPIO_PORT_B, .pin = 14},
  .sclk = {.port = GPIO_PORT_B, .pin = 13}, 
  .cs = {.port = GPIO_PORT_C, .pin = 0}, 
}; 

static const GpioAddress mems_int_1 = {.port = GPIO_PORT_C, .pin = 1};
static const GpioAddress mems_int_2 = {.port = GPIO_PORT_C, .pin = 2}; 

int main(void) {
  // Enable various peripherals
  interrupt_init();
  soft_timer_init();
  gpio_init();

  spi_init(SPI_PORT_2, &mems_spi_settings); 

  // Keep toggling the state of the pins from on to off with a 50 ms delay between.
  while (true) {

  }

}
