#include "uart.h"

#include "stm32f0xx_usart.h"

// Initializes USART with settings
StatusCode usart_init(USARTSettings *settings, USART_TypeDef *uart, uart, USART_InitTypeDef* uart_init) {
  uart_init->USART_BaudRate = 0x0000FFFF | (uint32_t)settings->baud_rate;
  uart_init->USART_WordLength = ~0x10001000; 	// 8 bit word len
  uart_init->USART_StopBits = 0x00003000;		// 1 stop bit
  uart_init->USART_Parity = ~0x00000400; 		// parity state disabled
  uart_init->USART_Mode = ;
  uart_init->USART_HardwareFlowControl = ;

  USART_Init(uart, uart_init);
}

// Transmits data
StatusCode usart_transmit(USART_TypeDef *uart, uint8_t data) {
  while(uart_buffer_full);
  write_into_buffer(data, tx_buffer);
  USART_SendData(USART, (uint16_t)data | 0x0100); // send data and start bit
}

// Tranmits array of data
StatusCode usart_transmit_array(USART_TypeDef *uart, uint8_t *data, uint8_t length) {
  extern uint8_t i;
  for(i = 0; i < length; i++) {
    uart_send(uart, data[i]);
  }
}

// Receives data
uint8_t usart_receive(USART_TypeDef uart) {
  return (uint8_t)(USART_ReceiveData(uart) & (uint16_t)0x00FF);
}

bool write_into_buffer(uint8_t data, RingBuffer *ring_buffer) {
  if(check_buffer_ptr(ring_buffer)){
    *ring_buffer->write = data;
    return advance_buffer_ptr(ring_buffer);
  }
  return false;
}

uint8_t read_from_buffer(RingBuffer *ring_buffer) {
  if(check_buffer_ptr(ring_buffer)){
      return *ring_buffer->write;
      advance_buffer_ptr(ring_buffer);
  }
  return false;
}