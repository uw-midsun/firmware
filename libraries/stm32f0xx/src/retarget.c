#include "retarget.h"
#include "stm32f0xx.h"

static void send_command(int command, void *message) {
  __ASM volatile(
    "mov r0, %[cmd]\n"
    "mov r1, %[msg]\n"
    "bkpt #0xAB\n"
    :
    : [cmd] "r" (command), [msg] "r" (message)
    : "r0", "r1", "memory"
  );
}

int _write(int fd, char *ptr, int len) {
  uint32_t m[] = {
    1, // stdout
    (uint32_t)ptr,
    len
  };

  send_command(0x05, m);

  return len;
}

void HardFault_Handler(void) {
  __ASM volatile("BKPT #01");
  while (1);
}

void retarget_init(void) {
  // Initialize UART
}