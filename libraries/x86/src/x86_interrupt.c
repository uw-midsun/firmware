#include "x86_interrupt.h"

#include <signal.h>
#include <stdbool.h>
#include <stdint.h>
#include <unistd.h>

#include "interrupt_def.h"
#include "status.h"

#define NUM_X86_INTERRUPT_HANDLERS 64
#define NUM_X86_INTERRUPT_INTERRUPTS 128

typedef struct Interrupt {
  InterruptPriority priority;
  uint8_t handler_id;
  bool is_event;
} Interrupt;

static pid_t s_main_thread_id = 0;

static uint8_t s_x86_interrupt_next_interrupt_id = 0;
static uint8_t s_x86_interrupt_next_handler_id = 0;

static Interrupt s_x86_interrupt_interrupts_map[NUM_X86_INTERRUPT_INTERRUPTS];
static x86_interrupt_handler s_x86_interrupt_handlers[NUM_X86_INTERRUPT_HANDLERS];

// Signal handler for all interrupts. Prioritization is handled by the implementation of signals and
// the init function. Signals of higher priority interrupt the running of this function. All other
// signals are stored in a pqueue and are executed in order of priority then arrival. Runs the
// handler associated with the interrupt id it recieves via the sival_int.
void prv_sig_handler(int signum, siginfo_t *info, void *ptr) {
  if (info->si_value.sival_int < NUM_X86_INTERRUPT_INTERRUPTS) {
    // If the interrupt is an event don't run the handler as it is just a wake event.
    if (!s_x86_interrupt_interrupts_map[info->si_value.sival_int].is_event) {
      // Execute the handler passing it the interrupt ID. To determine which handler look up in
      // the interrupts map by interrupt ID.
      s_x86_interrupt_handlers[s_x86_interrupt_interrupts_map[info->si_value.sival_int].handler_id](
          info->si_value.sival_int);
    }
  }
}

void x86_interrupt_init(void) {
  // Assign the s_main_thread_id to be the thread "owning" the interrupts. This prevents
  // subprocesses from sending a signal to itself.
  s_main_thread_id = getpid();

  // Create a generic sigaction.
  struct sigaction act;
  act.sa_sigaction = prv_sig_handler;
  act.sa_flags = SA_SIGINFO;

  // Define an empty blocking mask (no signals are blocked).
  sigset_t block_mask;
  sigemptyset(&block_mask);

  // Add a rule for low priority interrupts which blocks only other low priority signals.
  sigaddset(&block_mask, SIGRTMIN + INTERRUPT_PRIORITY_LOW);
  act.sa_mask = block_mask;
  sigaction(SIGRTMIN + INTERRUPT_PRIORITY_LOW, &act, NULL);

  // Add a rule for normal priority interrupts which blocks low and other normal priority signals.
  sigaddset(&block_mask, SIGRTMIN + INTERRUPT_PRIORITY_NORMAL);
  act.sa_mask = block_mask;
  sigaction(SIGRTMIN + INTERRUPT_PRIORITY_NORMAL, &act, NULL);

  // Add a rule for high priority interrupts which blocks all other interrupt signals.
  sigaddset(&block_mask, SIGRTMIN + INTERRUPT_PRIORITY_HIGH);
  act.sa_mask = block_mask;
  sigaction(SIGRTMIN + INTERRUPT_PRIORITY_HIGH, &act, NULL);

  s_x86_interrupt_next_interrupt_id = 0;
  s_x86_interrupt_next_handler_id = 0;

  memset(&s_x86_interrupt_interrupts_map, 0, sizeof(s_x86_interrupt_interrupts_map));
  memset(&s_x86_interrupt_handlers, 0, sizeof(s_x86_interrupt_handlers));
}

StatusCode x86_interrupt_register_handler(x86_interrupt_handler handler, uint8_t *handler_id) {
  if (s_x86_interrupt_next_handler_id >= NUM_X86_INTERRUPT_HANDLERS) {
    return status_code(STATUS_CODE_RESOURCE_EXHAUSTED);
  }

  *handler_id = s_x86_interrupt_next_handler_id;
  s_x86_interrupt_next_handler_id++;
  s_x86_interrupt_handlers[*handler_id] = handler;

  return STATUS_CODE_OK;
}

StatusCode x86_interrupt_register_interrupt(uint8_t handler_id, InterruptSettings *settings,
                                            uint8_t *interrupt_id) {
  if (handler_id >= s_x86_interrupt_next_handler_id ||
      settings->priority >= NUM_INTERRUPT_PRIORITY || settings->type >= NUM_INTERRUPT_TYPE) {
    return status_code(STATUS_CODE_INVALID_ARGS);
  } else if (s_x86_interrupt_next_interrupt_id >= NUM_X86_INTERRUPT_INTERRUPTS) {
    return status_code(STATUS_CODE_RESOURCE_EXHAUSTED);
  }

  *interrupt_id = s_x86_interrupt_next_interrupt_id;
  s_x86_interrupt_next_interrupt_id++;
  Interrupt interrupt = {
    .priority = settings->priority, .handler_id = handler_id, .is_event = (bool)settings->type
  };
  s_x86_interrupt_interrupts_map[*interrupt_id] = interrupt;

  return STATUS_CODE_OK;
}

StatusCode x86_interrupt_trigger(uint8_t interrupt_id) {
  if (interrupt_id >= s_x86_interrupt_next_interrupt_id) {
    return status_code(STATUS_CODE_INVALID_ARGS);
  }

  // Enqueue a new signal sent to this process that has a signal number determined by the id for the
  // callback it is going to run.
  siginfo_t value_store;
  value_store.si_value.sival_int = interrupt_id;
  sigqueue(s_main_thread_id, SIGRTMIN + s_x86_interrupt_interrupts_map[interrupt_id].priority,
           value_store.si_value);

  return STATUS_CODE_OK;
}
