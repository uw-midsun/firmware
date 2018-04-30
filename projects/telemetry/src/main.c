#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include "log.h"

#include "delay.h"
#include "gpio.h"  // General Purpose I/O control.
#include "gps.h"
#include "interrupt.h"   // For enabling interrupts.
#include "soft_timer.h"  // Software timers for scheduling future events.
#include "uart.h"

// As far as I can tell we just made up the values below
static const GPIOAddress pins[] = {
  { .port = GPIO_PORT_B, .pin = 3 },  // Pin power
  { .port = GPIO_PORT_B, .pin = 4 },  // Pin on_off
};

void gga_handler(evm_gps_gga_sentence result) {
  // printf("r.north_south: %c\n", result.north_south);
}

// For GPS
static const UARTPort s_port = UART_PORT_3;

int main(void) {
  printf("Starting main\n");
  // Enable various peripherals
  interrupt_init();
  gpio_init();
  soft_timer_init();

  UARTSettings s_settings = {
    .baudrate = 9600,

    .tx = { .port = GPIO_PORT_A, .pin = 10 },
    .rx = { .port = GPIO_PORT_A, .pin = 10 },
    .alt_fn = GPIO_ALTFN_4,
  };

  const GPIOSettings settings_power = {
    .direction = GPIO_DIR_OUT,  // The pin needs to output.
    .state = GPIO_STATE_LOW,    // Start in the "off" state.
    .alt_function = GPIO_ALTFN_NONE,
  };

  const GPIOSettings settings_on_off = {
    .direction = GPIO_DIR_OUT,  // The pin needs to output.
    .state = GPIO_STATE_LOW,    // Start in the "off" state.
    .alt_function = GPIO_ALTFN_NONE,
  };

  evm_gps_settings settings = { .settings_power = &settings_power,
                                .settings_on_off = &settings_on_off,
                                .pin_power = &pins[0],
                                .pin_on_off = &pins[1],
                                .uart_settings = &s_settings,
                                .port = s_port };
  StatusCode ret = evm_gps_init(&settings);
  // printf("evm_gps_init returned with StatusCode: %d\n", ret);
  // ret = evm_gps_add_gga_handler(gga_handler, NULL);
  // printf("evm_gps_add_gga_handler returned with StatusCode: %d\n", ret);

  while (true) {
    // printf("%s","");
    // delay_s(1);
  }
  evm_gps_clean_up(&settings);
  return 0;
}
