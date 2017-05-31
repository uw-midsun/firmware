#include "uart.h"
#include "stm32f0xx.h"

// basic idea: tx is stored in a buffer, interrupt-driven
// rx is buffered, once a newline is hit or the buffer is full, call rx_handler

// TODO: move into separate source? Maybe these should be in a "board config"-style file
typedef struct {
  const void (*rcc_cmd)(uint32_t periph, FunctionalState state);
  const uint32_t periph;
  const USART_TypeDef *base;
  UARTStorage *storage;
} UARTPortData;

static UARTPortData s_port[] = {
  {
    .rcc_cmd = RCC_APB2PeriphClockCmd,
    .periph = RCC_APB2Periph_USART1,
    .base = USART1
  },
  {
    .rcc_cmd = RCC_APB1PeriphClockCmd,
    .periph = RCC_APB1Periph_USART2,
    .base = USART2
  },
  {
    .rcc_cmd = RCC_APB1PeriphClockCmd,
    .periph = RCC_APB1Periph_USART3,
    .base = USART3
  },
  {
    .rcc_cmd = RCC_APB1PeriphClockCmd,
    .periph = RCC_APB1Periph_USART4,
    .base = USART4
  }
};

static void prv_tx_pop(UARTPort uart);

static void prv_rx_push(UARTPort uart);

StatusCode uart_init(UARTPort uart, UARTSettings *settings, UARTStorage *storage) {
  s_port[uart].rcc_cmd(s_port[uart].periph, ENABLE);

  s_port[uart].storage = storage;
  memset(storage, 0, sizeof(*storage));

  s_port[uart].storage->rx_handler = settings->rx_handler;
  s_port[uart].storage->context = settings->context;
  fifo_init(&s_port[uart].storage->tx_fifo, s_port[uart].storage->tx_buf);
  fifo_init(&s_port[uart].storage->rx_fifo, s_port[uart].storage->rx_buf);

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

  USART_ITConfig(s_port[uart].base, USART_IT_TC, ENABLE);
  USART_ITConfig(s_port[uart].base, USART_IT_RXNE, ENABLE);

  USART_Cmd(s_port[uart].base, ENABLE);
}

StatusCode uart_tx(UARTPort uart, uint8_t *tx_data, size_t len) {
  status_ok_or_return(fifo_push_arr(&s_port[uart].storage->tx_fifo, tx_data, len));

  // TODO: if no transfer is currently happening
  if (USART_GetITStatus(s_port[uart].base, USART_IT_TXE) == SET) {
    prv_tx_pop(uart);
  }

  return STATUS_CODE_OK;
}

static void prv_tx_pop(UARTPort uart) {
  if (fifo_size(&s_port[uart].storage->tx_fifo) > 0) {
    uint8_t tx_data = 0;
    fifo_pop(&s_port[uart].storage->tx_fifo, &tx_data);

    USART_SendData(s_port[uart].base, tx_data);
  }
}

static void prv_rx_push(UARTPort uart) {
  uint8_t rx_data = USART_ReceiveData(s_port[uart].base);
  fifo_push(&s_port[uart].storage->rx_fifo, &rx_data);

  size_t num_bytes = fifo_size(&s_port[uart].storage->rx_fifo);
  if (rx_data == '\n' || num_bytes == UART_MAX_BUFFER_LEN) {
    uint8_t buf[UART_MAX_BUFFER_LEN] = { 0 };
    fifo_pop_arr(&s_port[uart].storage->rx_fifo, buf, num_bytes);

    s_port[uart].storage->rx_handler(buf, num_bytes, s_port[uart].storage->context);
  }
}

void USART1_IRQHandler(void) {

}

void USART2_IRQHandler(void) {
  UARTPort port = 1;

  if (USART_GetITStatus(s_port[port].base, USART_IT_TC) == SET) {
    prv_tx_pop(port);
  }

  if (USART_GetITStatus(s_port[port].base, USART_IT_RXNE) == SET) {
    prv_rx_push(port);
  }
}

void USART3_4_IRQHandler(void) {

}
