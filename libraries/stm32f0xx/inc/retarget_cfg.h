#pragma once

#define RETARGET_CFG_UART USART1
#define RETARGET_CFG_UART_ENABLE_CLK()                                         \
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE)

#define RETARGET_CFG_UART_GPIO_PORT GPIOB
#define RETARGET_CFG_UART_GPIO_TX 6
#define RETARGET_CFG_UART_GPIO_RX 7
#define RETARGET_CFG_UART_GPIO_ALTFN GPIO_AF_0
#define RETARGET_CFG_UART_GPIO_ENABLE_CLK()                                    \
  RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOB, ENABLE)
