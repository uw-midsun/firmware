// Retargets STDOUT to semihosting if SEMIHOSTING_ENABLED=1.
// Note that semihosting will cause devices that aren't connected to a debugger to hang.
// TODO(ELEC-36): Replace semihosting with UART
#include "retarget.h"
#include "stm32f0xx.h"

// Send a command to OpenOCD - semihosting must be enabled
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

#if SEMIHOSTING_ENABLED
  // Only retarget to semihosting if explicitly enabled since
  // our hard fault handler doesn't play nicely with semihosting
  send_command(0x05, m);
#endif

  return len;
}

void HardFault_Handler(void) {
  __ASM volatile("BKPT #01");
  while (1);
}

void retarget_init(void) {
  // Stub for UART initialization
}
