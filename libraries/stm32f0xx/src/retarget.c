#include "retarget.h"
#include "stm32f0xx.h"

#define min(a,b) \
  ({ __typeof__ (a) _a = (a); \
     __typeof__ (b) _b = (b); \
    _a <= _b ? _a : _b; })

#define RETARGET_BUFFER_SIZE 1000

char gv_buffer[RETARGET_BUFFER_SIZE] = { 0 };
int gv_pos = 0;

int _write(int fd, char *ptr, int len) {
  int space = min(len, RETARGET_BUFFER_SIZE - gv_pos - len);
  memcpy(gv_buffer + gv_pos, ptr, space);
  gv_pos += space;

  return space;
}

void HardFault_Handler(void) {
  __ASM volatile("BKPT #01");
  while (1);
}

void retarget_init(void) {
  // Initialize UART
}