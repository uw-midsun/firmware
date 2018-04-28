#include "x86_interrupt.h"

#include <pthread.h>
#include <semaphore.h>
#include <signal.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>

#include "interrupt_def.h"
#include "log.h"
#include "status.h"

#define NUM_X86_INTERRUPT_HANDLERS 64
#define NUM_X86_INTERRUPT_INTERRUPTS 128

typedef enum {
  X86_INTERRUPT_STATE_NONE = 0,
  X86_INTERRUPT_STATE_MASK,
  X86_INTERRUPT_STATE_UNMASK,
} X86InterruptState;

typedef struct Interrupt {
  InterruptPriority priority;
  uint8_t handler_id;
  bool is_event;
} Interrupt;

static pthread_t s_sig_handler_thread;
static pthread_mutex_t s_keep_alive = PTHREAD_MUTEX_INITIALIZER;

X86InterruptState s_interrupt_state_update;
static pthread_cond_t s_block_mask_update = PTHREAD_COND_INITIALIZER;
static pthread_mutex_t s_block_interrupts = PTHREAD_MUTEX_INITIALIZER;

static pid_t s_pid = 0;

static uint8_t s_x86_interrupt_next_interrupt_id = 0;
static uint8_t s_x86_interrupt_next_handler_id = 0;

static Interrupt s_x86_interrupt_interrupts_map[NUM_X86_INTERRUPT_INTERRUPTS];
static x86InterruptHandler s_x86_interrupt_handlers[NUM_X86_INTERRUPT_HANDLERS];

// Signal handler for all interrupts. Prioritization is handled by the implementation of signals and
// the init function. Signals of higher priority interrupt the running of this function. All other
// signals are stored in a pqueue and are executed in order of priority then arrival. Runs the
// handler associated with the interrupt id it receives via the sival_int.
static void prv_sig_handler(int signum, siginfo_t *info, void *ptr) {
  LOG_DEBUG("%ld\n", pthread_self());
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

// Signal handler thread. This catches all signals on this thread to ensure deadlocks and other
// unsafe conditions are avoided.
static void *prv_sig_handler_thread(void *arg) {
  (void)arg;
  LOG_DEBUG("Starting Signal Handler (id:%ld)\n", pthread_self());
  // Create a handler sigaction.
  struct sigaction act;
  act.sa_sigaction = prv_sig_handler;
  act.sa_flags = SA_SIGINFO | SA_RESTART;  // Set SA_RESTART to allow syscalls to be retried.

  // Define an empty blocking mask (no signals are blocked to start).
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

  // Unblock the relevant signals on this thread.
  struct sched_param params;
  params.sched_priority = sched_get_priority_max(SCHED_FIFO);
  pthread_setschedparam(s_sig_handler_thread, SCHED_FIFO, &params);
  pthread_sigmask(SIG_UNBLOCK, &block_mask, NULL);

  // While in keep alive (i.e. not re-initing just run forever).
  while (pthread_mutex_trylock(&s_keep_alive) != 0) {
    // Use a condition variable to signal critical sections.
    pthread_mutex_lock(&s_block_interrupts);
    while (!s_interrupt_state_update) {
      pthread_cond_wait(&s_block_mask_update, &s_block_interrupts);
    }
    // Handle the update by altering the mask accordingly.
    if (s_interrupt_state_update == X86_INTERRUPT_STATE_MASK) {
      pthread_sigmask(SIG_BLOCK, &block_mask, NULL);
    } else if (s_interrupt_state_update == X86_INTERRUPT_STATE_UNMASK) {
      pthread_sigmask(SIG_UNBLOCK, &block_mask, NULL);
    }
    s_interrupt_state_update = X86_INTERRUPT_STATE_NONE;
    pthread_mutex_unlock(&s_block_interrupts);
  }
  return NULL;
}

void x86_interrupt_init(void) {
  LOG_DEBUG("Main Thread (id:%ld)\n", pthread_self());
  // If using interrupts block all signal on this thread and any spawned threads off of it.
  sigset_t block_mask;
  sigemptyset(&block_mask);
  sigaddset(&block_mask, SIGRTMIN + INTERRUPT_PRIORITY_LOW);
  sigaddset(&block_mask, SIGRTMIN + INTERRUPT_PRIORITY_NORMAL);
  sigaddset(&block_mask, SIGRTMIN + INTERRUPT_PRIORITY_HIGH);
  pthread_sigmask(SIG_BLOCK, &block_mask, NULL);

  // If the pid is set this function was called previously so cleanup the signal handler thread.
  if (s_pid) {
    pthread_mutex_unlock(&s_keep_alive);
    pthread_cond_signal(&s_block_mask_update);
    pthread_join(s_sig_handler_thread, NULL);
  }

  // Assign the s_pid to be the process "owning" the interrupts. This prevents
  // subprocesses from sending a signal to itself instead.
  s_pid = getpid();

  // Async setup.
  pthread_mutex_init(&s_keep_alive, NULL);
  pthread_mutex_lock(&s_keep_alive);
  pthread_mutex_init(&s_block_interrupts, NULL);
  pthread_cond_init(&s_block_mask_update, NULL);
  s_interrupt_state_update = X86_INTERRUPT_STATE_NONE;

  // Spawn the signal handler.
  pthread_create(&s_sig_handler_thread, NULL, prv_sig_handler_thread, NULL);

  // Clear statics.
  s_x86_interrupt_next_interrupt_id = 0;
  s_x86_interrupt_next_handler_id = 0;
  memset(&s_x86_interrupt_interrupts_map, 0, sizeof(s_x86_interrupt_interrupts_map));
  memset(&s_x86_interrupt_handlers, 0, sizeof(s_x86_interrupt_handlers));
}

StatusCode x86_interrupt_register_handler(x86InterruptHandler handler, uint8_t *handler_id) {
  if (s_x86_interrupt_next_handler_id >= NUM_X86_INTERRUPT_HANDLERS) {
    return status_code(STATUS_CODE_RESOURCE_EXHAUSTED);
  }

  *handler_id = s_x86_interrupt_next_handler_id;
  s_x86_interrupt_next_handler_id++;
  s_x86_interrupt_handlers[*handler_id] = handler;

  return STATUS_CODE_OK;
}

StatusCode x86_interrupt_register_interrupt(uint8_t handler_id, const InterruptSettings *settings,
                                            uint8_t *interrupt_id) {
  if (handler_id >= s_x86_interrupt_next_handler_id ||
      settings->priority >= NUM_INTERRUPT_PRIORITIES || settings->type >= NUM_INTERRUPT_TYPES) {
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
  sigqueue(s_pid, SIGRTMIN + (int)s_x86_interrupt_interrupts_map[interrupt_id].priority,
           value_store.si_value);

  return STATUS_CODE_OK;
}

void x86_interrupt_mask(void) {
  pthread_mutex_lock(&s_block_interrupts);
  s_interrupt_state_update = X86_INTERRUPT_STATE_MASK;
  pthread_cond_signal(&s_block_mask_update);
  pthread_mutex_unlock(&s_block_interrupts);
}

void x86_interrupt_unmask(void) {
  pthread_mutex_lock(&s_block_interrupts);
  s_interrupt_state_update = X86_INTERRUPT_STATE_UNMASK;
  pthread_cond_signal(&s_block_mask_update);
  pthread_mutex_unlock(&s_block_interrupts);
}
