#include <stdint.h>
#include <stdbool.h>
#include <stdio.h> //???

#include <stm32f0xx.h> //or w/e its called

#include "stm32f0xx_usart.h"

struct UARTConfig {
  struct tx_pin;
  struct rx_pin;
	//
  // volatile struct buf *tx_buf;
  // volatile struct buf *rx_buf;
}

void set_baudrate(uint32_t br);

void uart_init(const /*?*/ struct UARTConfig);

void uart_transmit(struct UARTConfig *uart, uint8_t data);
void uart_transmit_array(const struct UARTConfig *uart, const uint8_t *data, uint8_t length);

uint8_t uart_receive(uint8_t UARTConfig *uart);
