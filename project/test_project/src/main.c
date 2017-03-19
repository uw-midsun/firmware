#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#include "gpio.h"
#include "uart.h"

int main(void) {
  GPIOSettings settings = {GPIO_DIR_OUT, GPIO_STATE_HIGH, GPIO_RES_NONE, GPIO_ALTFN_NONE};
  GPIOAddress addr[] = {{2, 6}, {2, 7}, {2, 8}, {2, 9}};
  const uint32_t addr_size = sizeof(addr) / sizeof(GPIOAddress);

  GPIOSettings tx_settings = { 0 };
  tx_settings.direction = GPIO_DIR_OUT;
  tx_settings.state = GPIO_STATE_LOW;
  tx_settings.resistor = GPIO_RES_NONE;
  tx_settings.alt_function = GPIO_ALTFN_1;
  GPIOAddress tx_addr = {0, 9};
  gpio_init_pint(&tx_addr, &tx_settings);

  GPIOSettings rx_settings = { 0 };
  rx_settings.direction = GPIO_DIR_IN;
  rx_settings.state = GPIO_STATE_LOW;
  rx_settings.resistor = GPIO_RES_NONE;
  rx_settings.alt_function = GPIO_ALTFN_1;
  GPIOAddress rx_addr = {0, 10};
  gpio_init_pin(&rx_addr, &rx_settings);


  gpio_init();

  volatile uint32_t i;
  for (i = 0; i < addr_size; i++) {
    gpio_init_pin(&addr[i], &settings);
  }

  volatile uint32_t interval = 0;
  volatile uint32_t j;

  USART_InitTypeDef uart_test_init = {0};
  UARTSettings uart_test_settings = {0};
  uart_init(&uart_test_settings, USART1, &uart_test_init);
  uint8_t sendthis = 0x01;
  uint8_t last_send = 1;

  while (true) {
    if(sendthis >= 0xFF){
      sendthis = 0x01;
    }
    else{
      sendthis++;
    }

    uart_send(USART1, sendthis);

    for (i = 0; i < 100000; i++) {
    }

    if(sendthis == uart_receive(USART1)) {
      gpio_set_pin_state(&addr[j], GPIO_STATE_HIGH);
    }
    else {
      gpio_set_pin_state(&addr[j], GPIO_STATE_LOW);
    }

    // for (j = 0; j < addr_size; j++) {
    //   gpio_toggle_state(&addr[j]);
    // }
    //
  }
}
