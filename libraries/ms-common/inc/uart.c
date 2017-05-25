// define cpu freq

// define uart baud rate

#include <stdint.h>
#include <stdbool.h>
#include <stdio.h> //???


#include <stm32f0xx.h> //or w/e its called

#include "uart.h"
#include "stm32f0xx_usart.h"

// extern uint8_t storage_byte; // global byte used for storage

// struct buf {
// 	uint8_t buf_data;
// 	uint8_t buf_fill;
// }

struct UARTConfig {
  struct tx_pin;
  struct rx_pin;
	//
  // volatile struct buf *tx_buf;
  // volatile struct buf *rx_buf;
}

// bool buf_full(volatile struct *buf) {
//   return(buf->buf_fill == 0xFF);		// buf_fill
// }
// void buf_clear(volatile struct *buf) {
// 	buf->buf_data &= 0x00;
// 	buf->buf_fill &= 0x00;
// }

// bool input_data(struct UARTConfig *uart, uint8_t data) {
// 	uint8_t digit = 0x80;
// 	extern uint8_t i;
// 	for(i = 0; i < 8; i++) {
// 		uart->tx_buf->buf_data |= data &= (digit >> i);
// 		if(uart->tx_buf->buf_fill >> i) {
// 			return false;
// 		}
// 		uart->tx_buf->buf_fill |= 0x01 << i;
// 	}
// 	return true;
// }

/*============================================================================*/

void set_baudrate(uint32_t br) {
	// storage_bit = USART_CR1 % 2; 	// store initial USART state (on/off)
	USART_CR1 &= ~UE; 								// ensure that clock is off
	USART_BRR = br;
	// USART_CR1 |= storage_bit; 		// restores initial USART state
}

void uart_init(const /*?*/ struct UARTConfig) {

	USART_CR1 &= ~UE;										// clk is off for UART, 1 start bit, 8 data bits
	USART_CR1 &= ~(M0 | M1); 						// 1 start bit, 8 data bits
	USART_CR2 &= ~(STOP[0] | STOP[1]); 	// 1 STOP bit

	GPIO_CLEAR_BIT(tx_pin);
	GPIO_SET_DIR(tx_pin, OUT);

	GPIO_CLEAR_BIT(rx_pin);
	GPIO_SET_DIR(rx_pin, IN);

  set_baudrate(9600);

	USART_CR1 &= ~(RE | TE | TXEIE | RXNEIE);
	USART_CR1 |= (TCIE);
}

void uart_transmit(struct UARTConfig *uart, uint8_t data) {
	while(USART_CR1 & TCIE);					 			// start transmission when previous transmit is complete
	USART_CR1 |= TE;												// set Transmit flag
	USART_TDR = data;												// put data onto tx buffer

	USART_CR1 &= ~TCIE;											// transmit in progress
	USART_CR1 |= TXEIE;											// transmit interrupt enabled
}

void uart_transmit_array(const struct UARTConfig *uart, const uint8_t *data, uint8_t length) {
  extern uint8_t i;
  for(i = 0; i < length; i++) {
  	uart_transmit(uart, data[i]);
  }
}

//
uint8_t uart_receive(uint8_t UARTConfig *uart) {
	USART |= RE; 														// set receive flag
	while(USART_CR1 & RXNE) {
		USART_CR1 |= RXNEIE;									// if the buffer isn't empty then call an interrupt
	}
	USART_CR1 &= ~RXNEIE;										// stop interrupt
	USART_CR1 &= ~RE;												// clear recive flag
	return USART_RDR;												// return contents of rx buffer
}
