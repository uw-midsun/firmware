#include "interrupt.h"

#include <signal.h>
#include <unistd.h>

#include "gpio.h"
#include "gpio_cfg.h"
#include "status.h"

#define NUM_INTERRUPTS 255
#define INTERRUPT_UNUSED 0

typedef struct interrupt {
  InterruptPriority priority;
  interrupt_callback callback;
} interrupt;

static interrupt s_interrupt_map[NUM_INTERRUPTS];
static uint8_t s_interrupt_next_id = 1;

// Signal handler for all interrupts. Prioritizing is handled by the implementation of signals and
// the init function. Signals of higher priority interrupt the running of this function. All other
// signals are stored in a pqueue and are executed in order of priority then arrival. Runs the
// callback for the callback with the id of sival_int.
void prv_sig_handler(int signum, siginfo_t *info, void *ptr) {
  if (info->si_value.sival_int < NUM_INTERRUPTS && info->si_value.sival_int != INTERRUPT_UNUSED) {
    s_interrupt_map[info->si_value.sival_int].callback();
  }
}

void interrupt_init() {
  // Create a generic sigaction.
  struct sigaction act;
  act.sa_sigaction = prv_sig_handler;
  act.sa_flags = SA_SIGINFO;

  // Define an empty blocking mask (no signals are blocked).
  sigset_t block_mask;
  sigemptyset(&block_mask);

  // Add a rule for low priority interrupts which block only other low priority signals.
  sigaddset(&block_mask, SIGRTMIN + INTERRUPT_PRIORITY_LOW);
  act.sa_mask = block_mask;
  sigaction(SIGRTMIN + INTERRUPT_PRIORITY_LOW, &act, NULL);

  // Add a rule for normal priority interrupts which block low and other normal priority signals.
  sigaddset(&block_mask, SIGRTMIN + INTERRUPT_PRIORITY_NORMAL);
  act.sa_mask = block_mask;
  sigaction(SIGRTMIN + INTERRUPT_PRIORITY_NORMAL, &act, NULL);

  // Add a rule for high priority interrupts which block all other interrupt signals.
  sigaddset(&block_mask, SIGRTMIN + INTERRUPT_PRIORITY_HIGH);
  act.sa_mask = block_mask;
  sigaction(SIGRTMIN + INTERRUPT_PRIORITY_HIGH, &act, NULL);
}

// Use this to count how many nested disables there are and only enable once out of all critical
// sections.
static uint8_t s_interrupt_priority_mask = 0;

void interrupt_enable() {
  if (s_interrupt_priority_mask > 0) {
    s_interrupt_priority_mask--;
  }
  if (!s_interrupt_priority_mask) {
    // Clear the block mask for this process (block no incoming signals).
    sigset_t block_mask;
    sigemptyset(&block_mask);
    sigprocmask(SIG_SETMASK, &block_mask, NULL);
  }
}

void interrupt_disable() {
  s_interrupt_priority_mask++;
  // Set a block mask for this process (block only incoming signals from the channels alotted for
  // interrupts).
  sigset_t mask;
  sigemptyset(&mask);
  sigaddset(&mask, SIGRTMIN + INTERRUPT_PRIORITY_LOW);
  sigaddset(&mask, SIGRTMIN + INTERRUPT_PRIORITY_NORMAL);
  sigaddset(&mask, SIGRTMIN + INTERRUPT_PRIORITY_HIGH);
  sigprocmask(SIG_SETMASK, &mask, NULL);
}

// REGISTRATION AND TRIGGER HELPERS
// Use these functions to register and trigger interrupts using module specific registration and
// trigger functions.

// Register an interrupt and assign it an id. When the interrupt is triggered via a software
// interrupt of external signal it must be referenced by this id the sival_int.
StatusCode prv_register_interrupt(InterruptSettings *settings, interrupt_callback callback,
                                  uint8_t *id) {
  if (s_interrupt_next_id >= NUM_INTERRUPTS) {
    return status_msg(STATUS_CODE_RESOURCE_EXHAUSTED, "Too many interrupts registered.");
  } else if (settings->type >= NUM_INTERRUPT_TYPE || settings->edge >= NUM_INTERRUPT_EDGE ||
             settings->priority >= NUM_INTERRUPT_PRIORITY) {
    return status_code(STATUS_CODE_INVALID_ARGS);
  }

  // Assign an interrupt to an array and give it a new id.
  s_interrupt_map[s_interrupt_next_id].priority = settings->priority;
  s_interrupt_map[s_interrupt_next_id].callback = callback;
  *id = s_interrupt_next_id++;

  return STATUS_CODE_OK;
}

// Trigger a software interrupt for the given interrupt by its id.
StatusCode prv_trigger_interrupt(uint8_t id) {
  if (id == INTERRUPT_UNUSED) {
    return status_code(STATUS_CODE_INVALID_ARGS);
  }

  // Enqueue a new signal sent to this process that has a signal number determined by the id for the
  // callback it is going to run.
  siginfo_t value_store;
  value_store.si_value.sival_int = id;
  sigqueue(getpid(), SIGRTMIN + s_interrupt_map[id].priority, value_store.si_value);
  return STATUS_CODE_OK;
}

// GPIO INTERRUPTS

#define NUM_GPIO_INTERRUPTS 16
static uint8_t s_interrupt_gpio_id_map[NUM_GPIO_INTERRUPTS];

StatusCode interrupt_gpio_register(GPIOAddress *address, InterruptSettings *settings,
                                   interrupt_callback callback) {
  if (address->pin >= NUM_GPIO_PORTS || address->port >= NUM_GPIO_PINS) {
    return status_code(STATUS_CODE_INVALID_ARGS);
  } else if (s_interrupt_gpio_id_map[address->pin] != INTERRUPT_UNUSED) {
    return status_msg(STATUS_CODE_RESOURCE_EXHAUSTED, "Interrupt for this pin is already used.");
  }

  status_ok_or_return(
      prv_register_interrupt(settings, callback, &s_interrupt_gpio_id_map[address->pin]));

  return STATUS_CODE_OK;
}

StatusCode interrupt_gpio_trigger(GPIOAddress *address) {
  if (address->pin >= NUM_GPIO_PORTS || address->port >= NUM_GPIO_PINS) {
    return status_code(STATUS_CODE_INVALID_ARGS);
  } else if (s_interrupt_gpio_id_map[address->pin] == INTERRUPT_UNUSED) {
    return status_msg(STATUS_CODE_INVALID_ARGS, "No interrupt set for this pin.");
  }

  status_ok_or_return(prv_trigger_interrupt(s_interrupt_gpio_id_map[address->pin]));

  return STATUS_CODE_OK;
}
