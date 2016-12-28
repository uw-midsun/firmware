#include "interrupt.h"

#include <signal.h>
#include <stdbool.h>
#include <unistd.h>

static bool s_interrupts_disabled = false;

void interrupt_enable(bool disabled_in_scope) {
  if (s_interrupts_disabled && disabled_in_scope) {
    // Clear the block mask for this process to allow signals to be processed. (They will queue when
    // disbaled).
    sigset_t block_mask;
    sigemptyset(&block_mask);
    sigprocmask(SIG_SETMASK, &block_mask, NULL);
    s_interrupts_disabled = false;
  }
}

bool interrupt_disable() {
  if (!s_interrupts_disabled) {
    s_interrupts_disabled = true;
    // Set a block mask for this process on the signals we are using as interrupts. Don't block all
    // signals so SIGTERM and SIGINT can still kill the process if it hangs or needs to be stopped.
    sigset_t block_mask;
    sigemptyset(&block_mask);
    sigaddset(&block_mask, SIGRTMIN + INTERRUPT_PRIORITY_HIGH);
    sigaddset(&block_mask, SIGRTMIN + INTERRUPT_PRIORITY_NORMAL);
    sigaddset(&block_mask, SIGRTMIN + INTERRUPT_PRIORITY_LOW);
    sigprocmask(SIG_SETMASK, &block_mask, NULL);
    // Interrupts were not previously disabled.
    return false;
  }
  // Interrupts were previously disabled.
  return true;
}
