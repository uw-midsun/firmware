#include "interrupt_mcu.h"

#include <signal.h>
#include <stdbool.h>
#include <stdint.h>
#include <unistd.h>

#include "interrupt.h"
#include "status.h"

#define NUM_INTERRUPT_MCU_HANDLERS 64
#define NUM_INTERRUPT_MCU_INTERRUPTS 128

typedef struct Interrupt {
  InterruptPriority priority;
  uint8_t handler_id;
  bool is_event;
} Interrupt;

static uint8_t s_interrupt_mcu_next_interrupt_id = 0;
static uint8_t s_interrupt_mcu_next_handler_id = 0;

static bool s_interrupt_mcu_initialized = false;
static Interrupt s_interrupt_mcu_interrupts_map[NUM_INTERRUPT_MCU_INTERRUPTS];
static interrupt_mcu_handler s_interrupt_mcu_handlers[NUM_INTERRUPT_MCU_HANDLERS];

// Signal handler for all interrupts. Prioritization is handled by the implementation of signals and
// the init function. Signals of higher priority interrupt the running of this function. All other
// signals are stored in a pqueue and are executed in order of priority then arrival. Runs the
// handler associated with the interrupt id it recieves via the sival_int.
void prv_sig_handler(int signum, siginfo_t *info, void *ptr) {
  if (info->si_value.sival_int < NUM_INTERRUPT_MCU_INTERRUPTS) {
    // If the interrupt is an event don't run the handler it needs to be polled.
    if (!s_interrupt_mcu_interrupts_map[info->si_value.sival_int].is_event) {
      s_interrupt_mcu_handlers[s_interrupt_mcu_interrupts_map[info->si_value.sival_int].handler_id](
          info->si_value.sival_int);
    }
  }
}

void interrupt_mcu_init(void) {
  if (!s_interrupt_mcu_initialized) {
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

    s_interrupt_mcu_initialized = true;
  }
}

StatusCode interrupt_mcu_register_handler(interrupt_mcu_handler handler, uint8_t *handler_id) {
  if (s_interrupt_mcu_next_handler_id >= NUM_INTERRUPT_MCU_HANDLERS) {
    return status_code(STATUS_CODE_RESOURCE_EXHAUSTED);
  }

  *handler_id = s_interrupt_mcu_next_handler_id;
  s_interrupt_mcu_next_handler_id++;
  s_interrupt_mcu_handlers[*handler_id] = handler;

  return STATUS_CODE_OK;
}

StatusCode interrupt_mcu_register_interrupt(uint8_t handler_id, InterruptSettings *settings,
                                            uint8_t *interrupt_id) {
  if (handler_id >= s_interrupt_mcu_next_handler_id ||
      settings->priority >= NUM_INTERRUPT_PRIORITY || settings->type >= NUM_INTERRUPT_PRIORITY) {
    return status_code(STATUS_CODE_INVALID_ARGS);
  } else if (s_interrupt_mcu_next_interrupt_id >= NUM_INTERRUPT_MCU_HANDLERS) {
    return status_code(STATUS_CODE_RESOURCE_EXHAUSTED);
  }

  *interrupt_id = s_interrupt_mcu_next_interrupt_id;
  s_interrupt_mcu_next_interrupt_id++;
  Interrupt interrupt = {
    .priority = settings->priority, .handler_id = handler_id, .is_event = (bool)settings->type
  };
  s_interrupt_mcu_interrupts_map[*interrupt_id] = interrupt;

  return STATUS_CODE_OK;
}

StatusCode interrupt_mcu_trigger(uint8_t interrupt_id) {
  if (interrupt_id >= s_interrupt_mcu_next_interrupt_id) {
    return status_code(STATUS_CODE_INVALID_ARGS);
  }

  // Enqueue a new signal sent to this process that has a signal number determined by the id for the
  // callback it is going to run.
  siginfo_t value_store;
  value_store.si_value.sival_int = interrupt_id;
  sigqueue(getpid(), SIGRTMIN + s_interrupt_mcu_interrupts_map[interrupt_id].priority,
           value_store.si_value);

  return STATUS_CODE_OK;
}
