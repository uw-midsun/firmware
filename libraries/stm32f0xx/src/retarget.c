#include "retarget.h"
#include "stm32f0xx.h"

#define min(x, y) (((x) <= (y)) ? (x) : (y))

volatile char gv_data[500];
volatile int gv_pos;

// TODO: putchar is not working
int _write(int fd, char *ptr, int len) {
  int space = min(len, 500 - gv_pos - len);
  memcpy(gv_data + gv_pos, ptr, space);
  gv_pos += space;

  return space;
}

void HardFault_Handler(void) {
  __ASM volatile("BKPT #01");
  while (1);
}

void retarget_init(void) {
  WWDG_DeInit();
  gv_pos = 0;
  // Initialize UART
}