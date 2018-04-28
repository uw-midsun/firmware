#include "sequencer.h"

#include <stddef.h>
#include <string.h>

#include "chaos_events.h"
#include "event_queue.h"
#include "fsm.h"
#include "log.h"
#include "misc.h"
#include "power_path.h"
#include "relay_id.h"

#define SEQUENCER_EMPTY_DATA 0
#define SEQUENCER_NO_OP \
  { .id = CHAOS_EVENT_NO_OP, .data = SEQUENCER_EMPTY_DATA }

typedef struct SequencerEventPair {
  Event expected;
  Event next;
  bool await;  // If true ignore events until expected rather than triggering error.
} SequencerEventPair;

typedef struct SequencerStorage {
  SequencerEventPair *events_arr;
  uint16_t num_events;
  uint16_t event_idx;
  uint8_t retries;
} SequencerStorage;

// Statics
static FSM s_sequencer_fsm;
static SequencerStorage s_storage;

// Event order declarations

// Emergency State Order:
// - Disable DCDC Monitoring
// - Open main power relay
// - Open solar master rear relay
// - Open solar master front relay
// - Open battery relay
// - Disable all unnecessary GPIO pins
static const SequencerEventPair s_emergency_events[] = {
  { .expected = SEQUENCER_NO_OP,
    .next = { .id = CHAOS_EVENT_MONITOR_DISABLE, .data = POWER_PATH_SOURCE_ID_DCDC },
    .await = false },
  { .expected = { .id = CHAOS_EVENT_MONITOR_DISABLE, .data = POWER_PATH_SOURCE_ID_DCDC },
    .next = { .id = CHAOS_EVENT_OPEN_RELAY, .data = RELAY_ID_MAIN_POWER },
    .await = false },
  { .expected = { .id = CHAOS_EVENT_OPEN_RELAY, .data = RELAY_ID_MAIN_POWER },
    .next = SEQUENCER_NO_OP,
    .await = false },
  { .expected = { .id = CHAOS_EVENT_RELAY_OPENED, .data = RELAY_ID_MAIN_POWER },
    .next = { .id = CHAOS_EVENT_OPEN_RELAY, .data = RELAY_ID_SOLAR_MASTER_REAR },
    .await = true },
  { .expected = { .id = CHAOS_EVENT_OPEN_RELAY, .data = RELAY_ID_SOLAR_MASTER_REAR },
    .next = SEQUENCER_NO_OP,
    .await = false },
  { .expected = { .id = CHAOS_EVENT_RELAY_OPENED, .data = RELAY_ID_SOLAR_MASTER_REAR },
    .next = { .id = CHAOS_EVENT_OPEN_RELAY, .data = RELAY_ID_SOLAR_MASTER_FRONT },
    .await = true },
  { .expected = { .id = CHAOS_EVENT_OPEN_RELAY, .data = RELAY_ID_SOLAR_MASTER_FRONT },
    .next = SEQUENCER_NO_OP,
    .await = false },
  { .expected = { .id = CHAOS_EVENT_RELAY_OPENED, .data = RELAY_ID_SOLAR_MASTER_FRONT },
    .next = { .id = CHAOS_EVENT_OPEN_RELAY, .data = RELAY_ID_BATTERY },
    .await = true },
  { .expected = { .id = CHAOS_EVENT_OPEN_RELAY, .data = RELAY_ID_BATTERY },
    .next = SEQUENCER_NO_OP,
    .await = false },
  { .expected = { .id = CHAOS_EVENT_RELAY_OPENED, .data = RELAY_ID_BATTERY },
    .next = { .id = CHAOS_EVENT_GPIO_EMERGENCY, .data = SEQUENCER_EMPTY_DATA },
    .await = false },
  { .expected = { .id = CHAOS_EVENT_GPIO_EMERGENCY, .data = SEQUENCER_EMPTY_DATA },
    .next = { .id = CHAOS_EVENT_SEQUENCE_EMERGENCY_DONE, .data = SEQUENCER_EMPTY_DATA },
    .await = false },
};

// Idle State Order:
// - Disable DCDC Monitoring
// - Open main power relay
// - Open solar master rear relay
// - Open solar master front relay
// - Open battery relay
// - Disable all unnecessary GPIO pins
static const SequencerEventPair s_idle_events[] = {
  { .expected = SEQUENCER_NO_OP,
    .next = { .id = CHAOS_EVENT_MONITOR_DISABLE, .data = POWER_PATH_SOURCE_ID_DCDC },
    .await = false },
  { .expected = { .id = CHAOS_EVENT_MONITOR_DISABLE, .data = POWER_PATH_SOURCE_ID_DCDC },
    .next = { .id = CHAOS_EVENT_OPEN_RELAY, .data = RELAY_ID_MAIN_POWER },
    .await = false },
  { .expected = { .id = CHAOS_EVENT_OPEN_RELAY, .data = RELAY_ID_MAIN_POWER },
    .next = SEQUENCER_NO_OP,
    .await = false },
  { .expected = { .id = CHAOS_EVENT_RELAY_OPENED, .data = RELAY_ID_MAIN_POWER },
    .next = { .id = CHAOS_EVENT_OPEN_RELAY, .data = RELAY_ID_SOLAR_MASTER_REAR },
    .await = true },
  { .expected = { .id = CHAOS_EVENT_OPEN_RELAY, .data = RELAY_ID_SOLAR_MASTER_REAR },
    .next = SEQUENCER_NO_OP,
    .await = false },
  { .expected = { .id = CHAOS_EVENT_RELAY_OPENED, .data = RELAY_ID_SOLAR_MASTER_REAR },
    .next = { .id = CHAOS_EVENT_OPEN_RELAY, .data = RELAY_ID_SOLAR_MASTER_FRONT },
    .await = true },
  { .expected = { .id = CHAOS_EVENT_OPEN_RELAY, .data = RELAY_ID_SOLAR_MASTER_FRONT },
    .next = SEQUENCER_NO_OP,
    .await = false },
  { .expected = { .id = CHAOS_EVENT_RELAY_OPENED, .data = RELAY_ID_SOLAR_MASTER_FRONT },
    .next = { .id = CHAOS_EVENT_OPEN_RELAY, .data = RELAY_ID_BATTERY },
    .await = true },
  { .expected = { .id = CHAOS_EVENT_OPEN_RELAY, .data = RELAY_ID_BATTERY },
    .next = SEQUENCER_NO_OP,
    .await = false },
  { .expected = { .id = CHAOS_EVENT_RELAY_OPENED, .data = RELAY_ID_BATTERY },
    .next = { .id = CHAOS_EVENT_GPIO_IDLE, .data = SEQUENCER_EMPTY_DATA },
    .await = false },
  { .expected = { .id = CHAOS_EVENT_GPIO_IDLE, .data = SEQUENCER_EMPTY_DATA },
    .next = { .id = CHAOS_EVENT_SEQUENCE_IDLE_DONE, .data = SEQUENCER_EMPTY_DATA },
    .await = false },
};

// Charge State Order:
// - Turn on charging GPIOs
// - Open battery relay
// - Open solar master rear relay
// - Open solar master front relay
// - Open main power relay
// - Enable DCDC monitoring
static const SequencerEventPair s_charge_events[] = {
  { .expected = SEQUENCER_NO_OP,
    .next = { .id = CHAOS_EVENT_GPIO_CHARGE, .data = SEQUENCER_EMPTY_DATA },
    .await = false },
  { .expected = { .id = CHAOS_EVENT_GPIO_CHARGE, .data = SEQUENCER_EMPTY_DATA },
    .next = { .id = CHAOS_EVENT_CLOSE_RELAY, .data = RELAY_ID_BATTERY },
    .await = false },
  { .expected = { .id = CHAOS_EVENT_CLOSE_RELAY, .data = RELAY_ID_BATTERY },
    .next = SEQUENCER_NO_OP,
    .await = false },
  { .expected = { .id = CHAOS_EVENT_RELAY_CLOSED, .data = RELAY_ID_BATTERY },
    .next = { .id = CHAOS_EVENT_CLOSE_RELAY, .data = RELAY_ID_SOLAR_MASTER_REAR },
    .await = true },
  { .expected = { .id = CHAOS_EVENT_CLOSE_RELAY, .data = RELAY_ID_SOLAR_MASTER_REAR },
    .next = SEQUENCER_NO_OP,
    .await = false },
  { .expected = { .id = CHAOS_EVENT_RELAY_CLOSED, .data = RELAY_ID_SOLAR_MASTER_REAR },
    .next = { .id = CHAOS_EVENT_CLOSE_RELAY, .data = RELAY_ID_SOLAR_MASTER_FRONT },
    .await = true },
  { .expected = { .id = CHAOS_EVENT_CLOSE_RELAY, .data = RELAY_ID_SOLAR_MASTER_FRONT },
    .next = SEQUENCER_NO_OP,
    .await = false },
  { .expected = { .id = CHAOS_EVENT_RELAY_CLOSED, .data = RELAY_ID_SOLAR_MASTER_FRONT },
    .next = { .id = CHAOS_EVENT_CLOSE_RELAY, .data = RELAY_ID_MAIN_POWER },
    .await = true },
  { .expected = { .id = CHAOS_EVENT_CLOSE_RELAY, .data = RELAY_ID_MAIN_POWER },
    .next = SEQUENCER_NO_OP,
    .await = false },
  { .expected = { .id = CHAOS_EVENT_RELAY_CLOSED, .data = RELAY_ID_MAIN_POWER },
    .next = { .id = CHAOS_EVENT_MONITOR_ENABLE, .data = POWER_PATH_SOURCE_ID_DCDC },
    .await = true },
  { .expected = { .id = CHAOS_EVENT_MONITOR_ENABLE, .data = POWER_PATH_SOURCE_ID_DCDC },
    .next = { .id = CHAOS_EVENT_SEQUENCE_CHARGE_DONE, .data = SEQUENCER_EMPTY_DATA },
    .await = false },
};

// Drive State Order:
// - Turn on drive GPIOs
// - Open battery relay
// - Open solar master rear relay
// - Open solar master front relay
// - Open main power relay
// - Enable DCDC monitoring
static const SequencerEventPair s_drive_events[] = {
  { .expected = SEQUENCER_NO_OP,
    .next = { .id = CHAOS_EVENT_GPIO_DRIVE, .data = SEQUENCER_EMPTY_DATA },
    .await = false },
  { .expected = { .id = CHAOS_EVENT_GPIO_DRIVE, .data = SEQUENCER_EMPTY_DATA },
    .next = { .id = CHAOS_EVENT_CLOSE_RELAY, .data = RELAY_ID_BATTERY },
    .await = false },
  { .expected = { .id = CHAOS_EVENT_CLOSE_RELAY, .data = RELAY_ID_BATTERY },
    .next = SEQUENCER_NO_OP,
    .await = false },
  { .expected = { .id = CHAOS_EVENT_RELAY_CLOSED, .data = RELAY_ID_BATTERY },
    .next = { .id = CHAOS_EVENT_CLOSE_RELAY, .data = RELAY_ID_SOLAR_MASTER_REAR },
    .await = true },
  { .expected = { .id = CHAOS_EVENT_CLOSE_RELAY, .data = RELAY_ID_SOLAR_MASTER_REAR },
    .next = SEQUENCER_NO_OP,
    .await = false },
  { .expected = { .id = CHAOS_EVENT_RELAY_CLOSED, .data = RELAY_ID_SOLAR_MASTER_REAR },
    .next = { .id = CHAOS_EVENT_CLOSE_RELAY, .data = RELAY_ID_SOLAR_MASTER_FRONT },
    .await = true },
  { .expected = { .id = CHAOS_EVENT_CLOSE_RELAY, .data = RELAY_ID_SOLAR_MASTER_FRONT },
    .next = SEQUENCER_NO_OP,
    .await = false },
  { .expected = { .id = CHAOS_EVENT_RELAY_CLOSED, .data = RELAY_ID_SOLAR_MASTER_FRONT },
    .next = { .id = CHAOS_EVENT_CLOSE_RELAY, .data = RELAY_ID_MAIN_POWER },
    .await = true },
  { .expected = { .id = CHAOS_EVENT_CLOSE_RELAY, .data = RELAY_ID_MAIN_POWER },
    .next = SEQUENCER_NO_OP,
    .await = false },
  { .expected = { .id = CHAOS_EVENT_RELAY_CLOSED, .data = RELAY_ID_MAIN_POWER },
    .next = { .id = CHAOS_EVENT_MONITOR_ENABLE, .data = POWER_PATH_SOURCE_ID_DCDC },
    .await = true },
  { .expected = { .id = CHAOS_EVENT_MONITOR_ENABLE, .data = POWER_PATH_SOURCE_ID_DCDC },
    .next = { .id = CHAOS_EVENT_SEQUENCE_DRIVE_DONE, .data = SEQUENCER_EMPTY_DATA },
    .await = false },
};

// FSM Setup
FSM_DECLARE_STATE(sequencer_state_emergency);
FSM_DECLARE_STATE(sequencer_state_idle);
FSM_DECLARE_STATE(sequencer_state_charge);
FSM_DECLARE_STATE(sequencer_state_drive);

FSM_STATE_TRANSITION(sequencer_state_emergency) {
  FSM_ADD_TRANSITION(CHAOS_EVENT_SEQUENCE_IDLE, sequencer_state_idle);
  FSM_ADD_TRANSITION(CHAOS_EVENT_SEQUENCE_EMERGENCY, sequencer_state_emergency);
  FSM_ADD_TRANSITION(CHAOS_EVENT_SEQUENCE_RESET, sequencer_state_emergency);
}

FSM_STATE_TRANSITION(sequencer_state_idle) {
  FSM_ADD_TRANSITION(CHAOS_EVENT_SEQUENCE_EMERGENCY, sequencer_state_emergency);
  FSM_ADD_TRANSITION(CHAOS_EVENT_SEQUENCE_CHARGE, sequencer_state_charge);
  FSM_ADD_TRANSITION(CHAOS_EVENT_SEQUENCE_DRIVE, sequencer_state_drive);
  FSM_ADD_TRANSITION(CHAOS_EVENT_SEQUENCE_RESET, sequencer_state_idle);
}

FSM_STATE_TRANSITION(sequencer_state_charge) {
  FSM_ADD_TRANSITION(CHAOS_EVENT_SEQUENCE_EMERGENCY, sequencer_state_emergency);
  FSM_ADD_TRANSITION(CHAOS_EVENT_SEQUENCE_IDLE, sequencer_state_idle);
  FSM_ADD_TRANSITION(CHAOS_EVENT_SEQUENCE_RESET, sequencer_state_charge);
}

FSM_STATE_TRANSITION(sequencer_state_drive) {
  FSM_ADD_TRANSITION(CHAOS_EVENT_SEQUENCE_EMERGENCY, sequencer_state_emergency);
  FSM_ADD_TRANSITION(CHAOS_EVENT_SEQUENCE_IDLE, sequencer_state_idle);
  FSM_ADD_TRANSITION(CHAOS_EVENT_SEQUENCE_RESET, sequencer_state_drive);
}

// FSM Transitions
static void prv_sequencer_state_emergency(FSM *fsm, const Event *e, void *context) {
  (void)fsm;
  (void)e;
  SequencerStorage *storage = context;
  storage->event_idx = 0;
  storage->num_events = SIZEOF_ARRAY(s_emergency_events);
  storage->events_arr = (SequencerEventPair *)s_emergency_events;
}

static void prv_sequencer_state_idle(FSM *fsm, const Event *e, void *context) {
  (void)fsm;
  (void)e;
  SequencerStorage *storage = context;
  storage->event_idx = 0;
  storage->num_events = SIZEOF_ARRAY(s_idle_events);
  storage->events_arr = (SequencerEventPair *)s_idle_events;
}

static void prv_sequencer_state_charge(FSM *fsm, const Event *e, void *context) {
  (void)fsm;
  (void)e;
  SequencerStorage *storage = context;
  storage->event_idx = 0;
  storage->num_events = SIZEOF_ARRAY(s_charge_events);
  storage->events_arr = (SequencerEventPair *)s_charge_events;
}

static void prv_sequencer_state_drive(FSM *fsm, const Event *e, void *context) {
  (void)fsm;
  (void)e;
  SequencerStorage *storage = context;
  storage->event_idx = 0;
  storage->num_events = SIZEOF_ARRAY(s_drive_events);
  storage->events_arr = (SequencerEventPair *)s_drive_events;
}

// Public interface
void sequencer_init(void) {
  memset(&s_storage, 0, sizeof(s_storage));
  fsm_state_init(sequencer_state_emergency, prv_sequencer_state_emergency);
  fsm_state_init(sequencer_state_idle, prv_sequencer_state_idle);
  fsm_state_init(sequencer_state_charge, prv_sequencer_state_charge);
  fsm_state_init(sequencer_state_drive, prv_sequencer_state_drive);
  fsm_init(&s_sequencer_fsm, "SequenceFSM", &sequencer_state_idle, &s_storage);
}

StatusCode sequencer_publish_next_event(const Event *previous_event) {
  // Filter to only handled events. Those in the range
  // |(NUM_CHAOS_EVENTS_CAN, NUM_CHAOS_EVENTS_FSM)|
  // in addition to the emergency sequence messages.
  if (previous_event->id <= NUM_CHAOS_EVENTS_CAN &&
      !(previous_event->id == CHAOS_EVENT_SEQUENCE_EMERGENCY ||
        previous_event->id == CHAOS_EVENT_SEQUENCE_EMERGENCY_DONE)) {
    // Not in the range of handled events. This isn't necessarily invalid but won't be handled so
    // return unknown.
    return status_code(STATUS_CODE_UNKNOWN);
  } else if (previous_event->id == CHAOS_EVENT_NO_OP) {
    // Skip no-op messages.
    return STATUS_CODE_OK;
  }

  // Try to transition if the event is a sequence transition.
  if (previous_event->id == CHAOS_EVENT_SEQUENCE_IDLE ||
      previous_event->id == CHAOS_EVENT_SEQUENCE_CHARGE ||
      previous_event->id == CHAOS_EVENT_SEQUENCE_DRIVE ||
      previous_event->id == CHAOS_EVENT_SEQUENCE_EMERGENCY ||
      previous_event->id == CHAOS_EVENT_SEQUENCE_RESET) {
    if (!fsm_process_event(&s_sequencer_fsm, previous_event)) {
      LOG_DEBUG("Err failed transition: %u %u\n", previous_event->id, previous_event->data);
      return status_code(STATUS_CODE_INVALID_ARGS);
    }
  }

  // Stop if there are no more events.
  if (s_storage.event_idx >= s_storage.num_events) {
    s_storage.retries = 0;
    return status_code(STATUS_CODE_RESOURCE_EXHAUSTED);
  }

  // Make a copy for readability. The compiler will likely elide this copy anyway.
  SequencerEventPair event_pair = s_storage.events_arr[s_storage.event_idx];

  // If the |s_event_idx == 0| we have no need to check |previous_event| just raise |next|.
  if (s_storage.event_idx == 0) {
    event_raise(event_pair.next.id, event_pair.next.data);
    s_storage.event_idx++;
    return STATUS_CODE_OK;
  }

  // If we aren't awaiting the next event any unmatched event is considered to have arrived out of
  // order and is treated as a fault.
  //
  // NOTE: this implicitly means any events in the range
  // |(NUM_CHAOS_EVENTS_CAN, NUM_CHAOS_EVENTS_FSM)| must originate from the sequencer unless the
  // expected event is marked for await!!! This is to ensure redundancy and allow retry attempts. If
  // this fails 3 times consecutively then a serious fault has occurred with Chaos. We enter the
  // emergency state as something terrible has happened. Realistically this is almost impossible and
  // would only be triggered by relays repeatedly failing to transition.

  // Check if |previous_event| matched |expected|.
  if (memcmp(previous_event, &event_pair.expected, sizeof(Event)) != 0) {
    if (!event_pair.await) {
      // We aren't unconditionally awaiting so apply retry logic.
      s_storage.retries++;
      if (s_storage.retries >= 3) {
        event_raise(CHAOS_EVENT_SEQUENCE_EMERGENCY, SEQUENCER_EMPTY_DATA);
        return status_msg(STATUS_CODE_INTERNAL_ERROR, "Consistent failure! Emergency");
      }
      event_raise(CHAOS_EVENT_SEQUENCE_RESET, SEQUENCER_EMPTY_DATA);
      return status_msg(STATUS_CODE_INTERNAL_ERROR, "Restarting Sequence");
    } else if (event_pair.await) {
      // We are awaiting expected unconditionally so just keep waiting. This should really only be
      // used by relay events which have their own fault handling logic.
      return STATUS_CODE_OK;
    }
  }

  // The |previous_event| matches |expected|. Raise |next| if it isn't a |NO_OP| and increment the
  // index.
  if (!(event_pair.next.id == CHAOS_EVENT_NO_OP && event_pair.next.data == SEQUENCER_EMPTY_DATA)) {
    event_raise(event_pair.next.id, event_pair.next.data);
  }
  s_storage.event_idx++;
  return STATUS_CODE_OK;
}
