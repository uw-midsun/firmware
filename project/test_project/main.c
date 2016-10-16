#include <stdint.h>
#include <stdbool.h>
#include "stm32f0xx.h"

#define LED_PORT (GPIOC)
#define LED_PIN (GPIO_Pin_6 | GPIO_Pin_7 | GPIO_Pin_8 | GPIO_Pin_9)

void toggle_led(void) {
  static bool toggled = false;
  if (toggled) {
    GPIO_SetBits(LED_PORT, LED_PIN);
  } else {
    GPIO_ResetBits(LED_PORT, LED_PIN);
  }

  toggled = !toggled;
}

int main(void) {
  // Enable GPIO Peripheral clock
  RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOC, ENABLE);

  GPIO_InitTypeDef GPIO_InitStructure;

  // Configure pin in output push/pull mode
  GPIO_InitStructure.GPIO_Pin = LED_PIN;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
  GPIO_Init(LED_PORT, &GPIO_InitStructure);

  int x = 0;

  while (true) {
    volatile uint32_t i = 0;
    for (i = 0; i < x; i++) { }
    toggle_led();

    x += 10;
  }
}
