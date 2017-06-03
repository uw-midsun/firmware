#include "uart.h"
#include "stm32f0xx.h"

// basic idea: tx is stored in a buffer, interrupt-driven
// rx is buffered, once a newline is hit or the buffer is full, call rx_handler

typedef struct {
  const void (*rcc_cmd)(uint32_t periph, FunctionalState state);
  const uint32_t periph;
  const uint32_t irq;
  const USART_TypeDef *base;
  UARTStorage *storage;
} UARTPortData;

static UARTPortData s_port[] = {
  {
    .rcc_cmd = RCC_APB2PeriphClockCmd,
    .periph = RCC_APB2Periph_USART1,
    .irq = USART1_IRQn,
    .base = USART1
  },
  {
    .rcc_cmd = RCC_APB1PeriphClockCmd,
    .periph = RCC_APB1Periph_USART2,
    .irq = USART2_IRQn,
    .base = USART2
  },
  {
    .rcc_cmd = RCC_APB1PeriphClockCmd,
    .periph = RCC_APB1Periph_USART3,
    .irq = USART3_4_IRQn,
    .base = USART3
  },
  {
    .rcc_cmd = RCC_APB1PeriphClockCmd,
    .periph = RCC_APB1Periph_USART4,
    .irq = USART3_4_IRQn,
    .base = USART4
  }
};

static void prv_tx_pop(UARTPort uart);
static void prv_rx_push(UARTPort uart);

static void prv_handle_irq(UARTPort uart);

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

  USART_ClearITPendingBit(s_port[uart].base, USART_FLAG_TXE);
  USART_ITConfig(s_port[uart].base, USART_IT_TXE, DISABLE);
  USART_ITConfig(s_port[uart].base, USART_IT_RXNE, ENABLE);

  stm32f0xx_interrupt_nvic_enable(s_port[uart].irq, INTERRUPT_PRIORITY_NORMAL);

  USART_Cmd(s_port[uart].base, ENABLE);
}

StatusCode uart_tx(UARTPort uart, uint8_t *tx_data, size_t len) {
  status_ok_or_return(fifo_push_arr(&s_port[uart].storage->tx_fifo, tx_data, len));

  if (USART_GetFlagStatus(s_port[uart].base, USART_FLAG_TXE) == SET) {
    prv_tx_pop(uart);
    USART_ITConfig(s_port[uart].base, USART_IT_TXE, ENABLE);
  }

  return STATUS_CODE_OK;
}

static void prv_tx_pop(UARTPort uart) {
  if (fifo_size(&s_port[uart].storage->tx_fifo) == 0) {
    USART_ITConfig(s_port[uart].base, USART_IT_TXE, DISABLE);
    return;
  }

  uint8_t tx_data = 0;
  fifo_pop(&s_port[uart].storage->tx_fifo, &tx_data);

  USART_SendData(s_port[uart].base, tx_data);
}

static void prv_rx_push(UARTPort uart) {
  uint8_t rx_data = USART_ReceiveData(s_port[uart].base);
  fifo_push(&s_port[uart].storage->rx_fifo, &rx_data);

  size_t num_bytes = fifo_size(&s_port[uart].storage->rx_fifo);
  if (rx_data == '\n' || num_bytes == UART_MAX_BUFFER_LEN) {
    uint8_t buf[UART_MAX_BUFFER_LEN + 1] = { 0 };
    fifo_pop_arr(&s_port[uart].storage->rx_fifo, buf, num_bytes);

    s_port[uart].storage->rx_handler(buf, num_bytes, s_port[uart].storage->context);
  }
}

static void prv_handle_irq(UARTPort uart) {
  if (USART_GetITStatus(s_port[uart].base, USART_IT_TXE) == SET) {
    prv_tx_pop(uart);
    USART_ClearITPendingBit(s_port[uart].base, USART_IT_TXE);
  }

  if (USART_GetITStatus(s_port[uart].base, USART_IT_RXNE) == SET) {
    prv_rx_push(uart);
  }
}

void USART1_IRQHandler(void) {
  prv_handle_irq(UART_PORT_1);
}

void USART2_IRQHandler(void) {
  prv_handle_irq(UART_PORT_2);
}

void USART3_4_IRQHandler(void) {
  prv_handle_irq(UART_PORT_3);
  prv_handle_irq(UART_PORT_4);
}
