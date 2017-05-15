// Retargets STDOUT to UART
#include "retarget.h"
#include "retarget_cfg.h"
#include "stm32f0xx.h"

static void prv_init_gpio(void) {
  RETARGET_CFG_UART_GPIO_ENABLE_CLK();

  GPIO_PinAFConfig(RETARGET_CFG_UART_GPIO_PORT,
                   RETARGET_CFG_UART_GPIO_TX,
                   RETARGET_CFG_UART_GPIO_ALTFN);
  GPIO_PinAFConfig(RETARGET_CFG_UART_GPIO_PORT,
                   RETARGET_CFG_UART_GPIO_RX,
                   RETARGET_CFG_UART_GPIO_ALTFN);

  GPIO_InitTypeDef gpio_init = {
    .GPIO_Pin = (1 << RETARGET_CFG_UART_GPIO_TX) | (1 << RETARGET_CFG_UART_GPIO_RX),
    .GPIO_Mode = GPIO_Mode_AF,
    .GPIO_Speed = GPIO_Speed_10MHz,
    .GPIO_OType = GPIO_OType_PP,
    .GPIO_PuPd = GPIO_PuPd_UP
  };

  GPIO_Init(RETARGET_CFG_UART_GPIO_PORT, &gpio_init);
}

void retarget_init(void) {
  RETARGET_CFG_UART_ENABLE_CLK();
  prv_init_gpio();

  USART_InitTypeDef usart_init;
  USART_StructInit(&usart_init);
  usart_init.USART_BaudRate = 115200;
  USART_Init(RETARGET_CFG_UART, &usart_init);

  USART_Cmd(RETARGET_CFG_UART, ENABLE);
}

int _write(int fd, char *ptr, int len) {
  for (int i = 0; i < len; i++) {
    while (USART_GetFlagStatus(RETARGET_CFG_UART, USART_FLAG_TXE) == RESET) { }
    USART_SendData(RETARGET_CFG_UART, (uint8_t)*(ptr + i));
  }

  return len;
}

void HardFault_Handler(void) {
  __ASM volatile("BKPT #01");
  while (1);
}
