#include "uart.h"
#include "stm32f0xx_usart.h"

// Initializes USART with settings
StatusCode uart_init(UARTSettings *settings, USART_TypeDef *uart, USART_InitTypeDef* uart_init) {
  uart_init->USART_BaudRate = 0x0000FFFF | (uint32_t)(settings->baud_rate);     //settings->baud_rate
  uart_init->USART_HardwareFlowControl = ~USART_CR3_CTSE;                       // hardware flow control disabled

  switch (settings->word_length) {
    case UART_WORD_LENGTH_7:
      uart_init->USART_WordLength = (USART_CR1_M << 16);
      break;
    case UART_WORD_LENGTH_8:
      uart_init->USART_WordLength = 0;
      break;
    case UART_WORD_LENGTH_9:
      uart_init->USART_WordLength = USART_CR1_M;
      break;
    default:
      uart_init->USART_WordLength = 0; 	                                        // 8 bit word len
  }
  switch (settings->stop_bits) {
    case UART_STOP_BITS_1:
      uart_init->USART_StopBits = 0;
    case UART_STOP_BITS_2:
      uart_init->USART_StopBits = USART_CR2_STOP_1;
    default:
      uart_init->USART_StopBits = 0;                                            // 1 stop bit
  }
  switch (settings->parity) {
    case UART_PARITY_DISABLE:
      uart_init->USART_Parity = 0;
    case UART_PARITY_EVEN:
      uart_init->USART_Parity = USART_CR1_PCE;
    case UART_PARITY_ODD:
    default:
      uart_init->USART_Parity = 0;                                              // parity state disabled
  }
  switch (settings->mode) {
    case UART_MODE_RXTX:
      uart_init->USART_Mode = USART_CR1_TE | USART_CR1_RE;
    case UART_MODE_RX:
      uart_init->USART_Mode = USART_CR1_RE;
    case UART_MODE_TX:
      uart_init->USART_Mode = USART_CR1_TE;
    default:
      uart_init->USART_Mode = USART_CR1_TE | USART_CR1_RE;                      // I no longer care about this
  }

  USART_Init(uart, uart_init);

  return STATUS_CODE_OK;
}

// Transmits data
StatusCode uart_send(USART_TypeDef *uart, uint8_t data) {
  // while(uart_buffer_full);
  // write_into_buffer(data, tx_buffer);
  USART_SendData(uart, (uint16_t)data | 0x0100); // send data and start bit
  return STATUS_CODE_OK;
}

// Tranmits array of data
StatusCode uart_send_array(USART_TypeDef *uart, uint8_t *data, uint8_t length) {
  extern uint8_t i;
  for(i = 0; i < length; i++) {
    uart_send(uart, data[i]);
  }
  return STATUS_CODE_OK;
}

// Receives data
uint8_t uart_receive(USART_TypeDef *uart) {
  return (uint8_t)(USART_ReceiveData(uart) & (uint16_t)0x00FF);
}

// bool write_into_buffer(uint8_t data, RingBuffer *ring_buffer) {
//   if(check_buffer_ptr(ring_buffer)){
//     *ring_buffer->write = data;
//     return advance_buffer_ptr(ring_buffer);
//   }
//   return false;
// }
//
// uint8_t read_from_buffer(RingBuffer *ring_buffer) {
//   if(check_buffer_ptr(ring_buffer)){
//       return *ring_buffer->write;
//       advance_buffer_ptr(ring_buffer);
//   }
//   return false;
// }
