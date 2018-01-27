#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include "log.h"

#include "gpio.h"  // General Purpose I/O control.
#include "gps.h"
#include "interrupt.h"   // For enabling interrupts.
#include "soft_timer.h"  // Software timers for scheduling future events.
#include "uart.h"
#include "delay.h"

// As far as I can tell we just made up the values below
static const GPIOAddress pins[] = {
  { .port = GPIO_PORT_A, .pin = 2 },  //
  { .port = GPIO_PORT_A, .pin = 3 },  //
  { .port = GPIO_PORT_B, .pin = 3 },  //
  { .port = GPIO_PORT_C, .pin = 3 },  //
};

void gga_handler(evm_gps_gga_sentence result) {
  LOG_DEBUG("r.north_south: %c\n", (char)result.north_south);
  return;
}

// For GPS
static const UARTPort s_port = UART_PORT_2;

int main(void) {
  LOG_DEBUG("Starting\n");
  // Enable various peripherals
  interrupt_init();
  soft_timer_init();
  gpio_init();

  // These structs below are placeholders, please update values as needed
  const GPIOSettings settings_tx = {
    .direction = GPIO_DIR_OUT,     // The pin needs to output.
    .state = GPIO_STATE_LOW,      // Start in the "off" state.
  };

  const GPIOSettings settings_rx = {
    .direction = GPIO_DIR_IN,      // The pin needs to input.
    .state = GPIO_STATE_LOW,      // Start in the "off" state.
  };

  const GPIOSettings settings_power = {
    .direction = GPIO_DIR_OUT,     // The pin needs to output.
    .state = GPIO_STATE_LOW,      // Start in the "off" state.
  };

  const GPIOSettings settings_on_off = {
    .direction = GPIO_DIR_OUT,     // The pin needs to output.
    .state = GPIO_STATE_LOW,      // Start in the "off" state.
  };

  UARTSettings s_settings = {
    .baudrate = 9600,

    .tx = { .port = GPIO_PORT_A, .pin = 2 },
    .rx = { .port = GPIO_PORT_A, .pin = 3 },
    .alt_fn = GPIO_ALTFN_1,
  };
  evm_gps_settings settings = { .settings_tx = &settings_tx,
                                .settings_rx = &settings_rx,
                                .settings_power = &settings_power,
                                .settings_on_off = &settings_on_off,
                                .pin_tx = &pins[0],
                                .pin_rx = &pins[1],
                                .pin_power = &pins[2],
                                .pin_on_off = &pins[3],
                                .uart_settings = &s_settings,
                                .port = &s_port };
  evm_gps_init(&settings);
  evm_gps_add_gga_handler(gga_handler, NULL);

  for(int i = 0; i < 50000; i++){
    delay_ms(1);
    LOG_DEBUG("Looping: %d\n", i);
  }
  evm_gps_clean_up(&settings);
  return 0;
}
