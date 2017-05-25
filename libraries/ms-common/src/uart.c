#include "uart.h"
#include "stm32f0xx.h"

// basic idea: tx is stored in a buffer, interrupt-driven
// rx is buffered, once a newline is hit or the buffer is full, call rx_handler

#define UART_MAX_BUFFER_LEN 512

// TODO: move into separate source? Maybe these should be in a "board config"-style file
typedef struct {
  const void (*rcc_cmd)(uint32_t periph, FunctionalState state);
  const uint32_t periph;
  const USART_TypeDef *base;

  UARTRxHandler rx_handler;
  void *context;
} UARTPortData;

static UARTPortData s_port[] = {
  {
    .rcc_cmd = RCC_APB1PeriphClockCmd,
    .periph = RCC_APB1Periph_USART1,
    .base = USART1
  },
  {
    .rcc_cmd = RCC_APB2PeriphClockCmd,
    .periph = RCC_APB2Periph_USART2,
    .base = USART2
  },
  {
    .rcc_cmd = RCC_APB2PeriphClockCmd,
    .periph = RCC_APB2Periph_USART3,
    .base = USART3
  },
  {
    .rcc_cmd = RCC_APB2PeriphClockCmd,
    .periph = RCC_APB2Periph_USART4,
    .base = USART4
  }
};

StatusCode uart_init(UARTPort uart, UARTSettings *settings) {
  s_port[uart].rcc_cmd(s_port[uart].periph, ENABLE);

  s_port[uart].rx_handler = settings->rx_handler;
  s_port[uart].context = settings->context;

  GPIOSettings gpio_settings = {
    .alt_function = settings->alt_fn,
    .resistor = GPIO_RES_PULLUP
  };

  gpio_init_pin(&settings->tx, &gpio_settings);
  gpio_init_pin(&settings->rx, &gpio_settings);

  USART_InitTypeDef usart_init;
  USART_StructInit(&usart_init);
  usart_init.USART_BaudRate = settings->baudrate;
  USART_Init(s_port[uart].base, &usart_init);

  USART_Cmd(s_port[uart].base, ENABLE);
}

StatusCode uart_tx(UARTPort uart, uint8_t *tx_data, size_t len) {
  // TODO: copy into buffer, use TX ISR to push data out
  for (int i = 0; i < len; i++) {
    while (USART_GetFlagStatus(s_port[uart].base, USART_FLAG_TXE) == RESET) { }
    USART_SendData(s_port[uart].base, tx_data[i]);
  }

  return STATUS_CODE_OK;
}

void USART1_IRQHandler(void) {

}

void USART2_IRQHandler(void) {
  // TODO: write generic function that:
  // read into buffer
  // if character == '\n' or buffer full
  // call rx handler
}

void USART3_4_IRQHandler(void) {

}
