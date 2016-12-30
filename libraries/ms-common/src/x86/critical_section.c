#include "critical_section.h"

#include <signal.h>
#include <stdbool.h>
#include <unistd.h>

#include "interrupt.h"

static bool s_interrupts_disabled = false;

bool critical_section_start(void) {
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
    // Interrupts got disabled.
    return true;
  }
  // Interrupts did not get disabled.
  return false;
}

void critical_section_end(bool disabled_in_scope) {
  if (s_interrupts_disabled && disabled_in_scope) {
    // Clear the block mask for this process to allow signals to be processed. (They will queue when
    // disbaled).
    sigset_t block_mask;
    sigemptyset(&block_mask);
    sigprocmask(SIG_SETMASK, &block_mask, NULL);
    s_interrupts_disabled = false;
  }
}
