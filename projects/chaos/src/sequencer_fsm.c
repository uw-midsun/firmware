#include "sequencer_fsm.h"

#include <stddef.h>
#include <string.h>

#include "chaos_events.h"
#include "event_queue.h"
#include "fsm.h"
#include "log.h"
#include "misc.h"
#include "power_path.h"
#include "relay_id.h"
#include "relay_retry_service.h"
#include "sequencer.h"

#define SEQUENCER_FSM_MAX_RETRIES 2
#define NUM_SEQUENCER_FSM_FILTERS 3

// Statics
static FSM s_sequencer_fsm;
static SequencerStorage s_storage;

static uint8_t s_retries = 0;
static bool s_pending_transition = false;
static uint16_t s_filtered_ids[NUM_SEQUENCER_FSM_FILTERS] = { 0 };

// Adds a filter on |id| to ignore it until it is cleared.
static bool prv_add_filter(uint16_t id) {
  for (size_t i = 0; i < NUM_SEQUENCER_FSM_FILTERS; i++) {
    if (s_filtered_ids[i] == NUM_CHAOS_EVENTS) {
      s_filtered_ids[i] = id;
      return true;
    }
  }
  return false;
}

// Removes all filters from |s_filtered_ids|
static void prv_clear_filters(void) {
  for (size_t i = 0; i < NUM_SEQUENCER_FSM_FILTERS; i++) {
    s_filtered_ids[i] = NUM_CHAOS_EVENTS;
  }
}

// Checks if |id| is filtered.
static bool prv_is_filtered_id(uint16_t id) {
  for (size_t i = 0; i < NUM_SEQUENCER_FSM_FILTERS; i++) {
    if (s_filtered_ids[i] == id) {
      return true;
    }
  }
  return false;
}

// Event order declarations
//
// The first event is a guard used to know when we have fully flushed the previous state.

// Emergency State Order:
// - Turn off the charger (if active)
// - Disable all unnecessary GPIO pins (this will auto-open all the non-battery relays)
// - Open motor relay (no-op to sync FSM)
// - Open solar master rear relay (no-op to sync FSM)
// - Open solar master front relay (no-op to sync FSM)
// - Disable DCDC Monitoring
// - Open battery relay (DCDC) (actual relay retries)
static const SequencerEventPair s_emergency_events[] = {
  { .raise = { .id = CHAOS_EVENT_SEQUENCE_EMERGENCY_START, .data = SEQUENCER_EMPTY_DATA },
    .response = SEQUENCER_NO_RESPONSE },
  { .raise = { .id = CHAOS_EVENT_CHARGER_OPEN, .data = SEQUENCER_EMPTY_DATA },
    .response = SEQUENCER_NO_RESPONSE },
  { .raise = { .id = CHAOS_EVENT_GPIO_EMERGENCY, .data = SEQUENCER_EMPTY_DATA },
    .response = SEQUENCER_NO_RESPONSE },
  { .raise = { .id = CHAOS_EVENT_OPEN_RELAY, .data = RELAY_ID_MOTORS },
    .response = SEQUENCER_NO_RESPONSE },
  { .raise = { .id = CHAOS_EVENT_OPEN_RELAY, .data = RELAY_ID_SOLAR_MASTER_REAR },
    .response = SEQUENCER_NO_RESPONSE },
  { .raise = { .id = CHAOS_EVENT_OPEN_RELAY, .data = RELAY_ID_SOLAR_MASTER_FRONT },
    .response = SEQUENCER_NO_RESPONSE },
  { .raise = { .id = CHAOS_EVENT_MONITOR_DISABLE, .data = POWER_PATH_SOURCE_ID_DCDC },
    .response = SEQUENCER_NO_RESPONSE },
  { .raise = { .id = CHAOS_EVENT_OPEN_RELAY, .data = RELAY_ID_BATTERY_SLAVE },
    .response = SEQUENCER_NO_RESPONSE },
  { .raise = { .id = CHAOS_EVENT_OPEN_RELAY, .data = RELAY_ID_BATTERY_MAIN },
    .response = SEQUENCER_NO_RESPONSE },
  { .raise = { .id = CHAOS_EVENT_SEQUENCE_EMERGENCY_DONE, .data = SEQUENCER_EMPTY_DATA },
    .response = SEQUENCER_NO_RESPONSE },
};

// Idle State Order:
// - Turn off the charger (if active)
// - Open all GPIO that shouldn't be on. (auto-opens all non-battery relays)
// - Open motor relay (no-op to sync FSM)
// - Open solar master rear relay (no-op to sync FSM)
// - Open solar master front relay (no-op to sync FSM)
// - Disable DCDC Monitoring
// - Open battery relay (DCDC)
static const SequencerEventPair s_idle_events[] = {
  { .raise = { .id = CHAOS_EVENT_SEQUENCE_IDLE_START, .data = SEQUENCER_EMPTY_DATA },
    .response = SEQUENCER_NO_RESPONSE },
  { .raise = { .id = CHAOS_EVENT_CHARGER_OPEN, .data = SEQUENCER_EMPTY_DATA },
    .response = SEQUENCER_NO_RESPONSE },
  { .raise = { .id = CHAOS_EVENT_GPIO_IDLE, .data = SEQUENCER_EMPTY_DATA },
    .response = SEQUENCER_NO_RESPONSE },
  { .raise = { .id = CHAOS_EVENT_OPEN_RELAY, .data = RELAY_ID_MOTORS },
    .response = { .id = CHAOS_EVENT_RELAY_OPENED, .data = RELAY_ID_MOTORS } },
  { .raise = { .id = CHAOS_EVENT_OPEN_RELAY, .data = RELAY_ID_SOLAR_MASTER_REAR },
    .response = { .id = CHAOS_EVENT_RELAY_OPENED, .data = RELAY_ID_SOLAR_MASTER_REAR } },
  { .raise = { .id = CHAOS_EVENT_OPEN_RELAY, .data = RELAY_ID_SOLAR_MASTER_FRONT },
    .response = { .id = CHAOS_EVENT_RELAY_OPENED, .data = RELAY_ID_SOLAR_MASTER_FRONT } },
  { .raise = { .id = CHAOS_EVENT_MONITOR_DISABLE, .data = POWER_PATH_SOURCE_ID_DCDC },
    .response = SEQUENCER_NO_RESPONSE },
  { .raise = { .id = CHAOS_EVENT_OPEN_RELAY, .data = RELAY_ID_BATTERY_SLAVE },
    .response = { .id = CHAOS_EVENT_RELAY_OPENED, .data = RELAY_ID_BATTERY_SLAVE } },
  { .raise = { .id = CHAOS_EVENT_OPEN_RELAY, .data = RELAY_ID_BATTERY_MAIN },
    .response = { .id = CHAOS_EVENT_RELAY_OPENED, .data = RELAY_ID_BATTERY_MAIN } },
  { .raise = { .id = CHAOS_EVENT_SEQUENCE_IDLE_DONE, .data = SEQUENCER_EMPTY_DATA },
    .response = SEQUENCER_NO_RESPONSE },
};

// Charge State Order:
// - Open all GPIO that shouldn't be on.
// - Open the motor relay (no-op to sync FSM)
// - Close battery relay (DCDC)
// - Enable DCDC monitoring
// - Turn on charging GPIOs
// - Close solar master rear relay
// - Close solar master front relay
// - Turn on the charger
static const SequencerEventPair s_charge_events[] = {
  { .raise = { .id = CHAOS_EVENT_SEQUENCE_CHARGE_START, .data = SEQUENCER_EMPTY_DATA },
    .response = SEQUENCER_NO_RESPONSE },
  { .raise = { .id = CHAOS_EVENT_GPIO_CHARGE_PRECONFIG, .data = SEQUENCER_EMPTY_DATA },
    .response = SEQUENCER_NO_RESPONSE },
  { .raise = { .id = CHAOS_EVENT_OPEN_RELAY, .data = RELAY_ID_MOTORS },
    .response = { .id = CHAOS_EVENT_RELAY_OPENED, .data = RELAY_ID_MOTORS } },
  { .raise = { .id = CHAOS_EVENT_CLOSE_RELAY, .data = RELAY_ID_BATTERY_MAIN },
    .response = { .id = CHAOS_EVENT_RELAY_CLOSED, .data = RELAY_ID_BATTERY_MAIN } },
  { .raise = { .id = CHAOS_EVENT_CLOSE_RELAY, .data = RELAY_ID_BATTERY_SLAVE },
    .response = { .id = CHAOS_EVENT_RELAY_CLOSED, .data = RELAY_ID_BATTERY_SLAVE } },
  { .raise = { .id = CHAOS_EVENT_MONITOR_ENABLE, .data = POWER_PATH_SOURCE_ID_DCDC },
    .response = SEQUENCER_NO_RESPONSE },
  { .raise = { .id = CHAOS_EVENT_GPIO_CHARGE, .data = SEQUENCER_EMPTY_DATA },
    .response = SEQUENCER_NO_RESPONSE },
  { .raise = { .id = CHAOS_EVENT_CLOSE_RELAY, .data = RELAY_ID_SOLAR_MASTER_REAR },
    .response = { .id = CHAOS_EVENT_RELAY_CLOSED, .data = RELAY_ID_SOLAR_MASTER_REAR } },
  { .raise = { .id = CHAOS_EVENT_CLOSE_RELAY, .data = RELAY_ID_SOLAR_MASTER_FRONT },
    .response = { .id = CHAOS_EVENT_RELAY_CLOSED, .data = RELAY_ID_SOLAR_MASTER_FRONT } },
  { .raise = { .id = CHAOS_EVENT_CHARGER_CLOSE, .data = SEQUENCER_EMPTY_DATA },
    .response = SEQUENCER_NO_RESPONSE },
  { .raise = { .id = CHAOS_EVENT_SEQUENCE_CHARGE_DONE, .data = SEQUENCER_EMPTY_DATA },
    .response = SEQUENCER_NO_RESPONSE },
};

// Drive State Order:
// - Turn off the charger
// - Open all GPIO that shouldn't be on.
// - Close battery relay (DCDC)
// - Enable DCDC monitoring
// - Turn on drive GPIOs
// - Close solar master rear relay
// - Close solar master front relay
// - Close motor relay
static const SequencerEventPair s_drive_events[] = {
  { .raise = { .id = CHAOS_EVENT_SEQUENCE_DRIVE_START, .data = SEQUENCER_EMPTY_DATA },
    .response = SEQUENCER_NO_RESPONSE },
  { .raise = { .id = CHAOS_EVENT_CHARGER_OPEN, .data = SEQUENCER_EMPTY_DATA },
    .response = SEQUENCER_NO_RESPONSE },
  { .raise = { .id = CHAOS_EVENT_GPIO_DRIVE_PRECONFIG, .data = SEQUENCER_EMPTY_DATA },
    .response = SEQUENCER_NO_RESPONSE },
  { .raise = { .id = CHAOS_EVENT_CLOSE_RELAY, .data = RELAY_ID_BATTERY_MAIN },
    .response = { .id = CHAOS_EVENT_RELAY_CLOSED, .data = RELAY_ID_BATTERY_MAIN } },
  { .raise = { .id = CHAOS_EVENT_CLOSE_RELAY, .data = RELAY_ID_BATTERY_SLAVE },
    .response = { .id = CHAOS_EVENT_RELAY_CLOSED, .data = RELAY_ID_BATTERY_SLAVE } },
  { .raise = { .id = CHAOS_EVENT_MONITOR_ENABLE, .data = POWER_PATH_SOURCE_ID_DCDC },
    .response = SEQUENCER_NO_RESPONSE },
  { .raise = { .id = CHAOS_EVENT_GPIO_DRIVE, .data = SEQUENCER_EMPTY_DATA },
    .response = SEQUENCER_NO_RESPONSE },
  { .raise = { .id = CHAOS_EVENT_CLOSE_RELAY, .data = RELAY_ID_SOLAR_MASTER_REAR },
    .response = { .id = CHAOS_EVENT_RELAY_CLOSED, .data = RELAY_ID_SOLAR_MASTER_REAR } },
  { .raise = { .id = CHAOS_EVENT_CLOSE_RELAY, .data = RELAY_ID_SOLAR_MASTER_FRONT },
    .response = { .id = CHAOS_EVENT_RELAY_CLOSED, .data = RELAY_ID_SOLAR_MASTER_FRONT } },
  { .raise = { .id = CHAOS_EVENT_CLOSE_RELAY, .data = RELAY_ID_MOTORS },
    .response = { .id = CHAOS_EVENT_RELAY_CLOSED, .data = RELAY_ID_MOTORS } },
  { .raise = { .id = CHAOS_EVENT_SEQUENCE_DRIVE_DONE, .data = SEQUENCER_EMPTY_DATA },
    .response = SEQUENCER_NO_RESPONSE },
};

// FSM Setup
FSM_DECLARE_STATE(sequencer_state_emergency);
FSM_DECLARE_STATE(sequencer_state_idle);
FSM_DECLARE_STATE(sequencer_state_charge);
FSM_DECLARE_STATE(sequencer_state_drive);

FSM_STATE_TRANSITION(sequencer_state_emergency) {
  FSM_ADD_TRANSITION(CHAOS_EVENT_SEQUENCE_EMERGENCY, sequencer_state_emergency);
  FSM_ADD_TRANSITION(CHAOS_EVENT_SEQUENCE_IDLE, sequencer_state_idle);
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
  FSM_ADD_TRANSITION(CHAOS_EVENT_SEQUENCE_DRIVE, sequencer_state_drive);
  FSM_ADD_TRANSITION(CHAOS_EVENT_SEQUENCE_IDLE, sequencer_state_idle);
  FSM_ADD_TRANSITION(CHAOS_EVENT_SEQUENCE_RESET, sequencer_state_charge);
}

FSM_STATE_TRANSITION(sequencer_state_drive) {
  FSM_ADD_TRANSITION(CHAOS_EVENT_SEQUENCE_EMERGENCY, sequencer_state_emergency);
  FSM_ADD_TRANSITION(CHAOS_EVENT_SEQUENCE_CHARGE, sequencer_state_charge);
  FSM_ADD_TRANSITION(CHAOS_EVENT_SEQUENCE_IDLE, sequencer_state_idle);
  FSM_ADD_TRANSITION(CHAOS_EVENT_SEQUENCE_RESET, sequencer_state_drive);
}

// Return true if setup succeeded. Return false if we are blocked waiting on an async event to
// finish. If this returns false the caller should exit immediately.
static bool prv_sequencer_setup_common(void) {
  if (sequencer_stop_awaiting(&s_storage)) {
    relay_retry_service_fail_fast();
    // We actually transitioned so just reset the state until it succeeds. Note this needs to be
    // normal priority to allow events to flush. This is to flush an event we are waiting for
    // (i.e. relay transition) this will take a max of 15 ms to succeed.
    event_raise(CHAOS_EVENT_SEQUENCE_RESET, SEQUENCER_EMPTY_DATA);
    s_pending_transition = true;
    return false;
  }
  s_pending_transition = false;
  s_retries = 0;
  prv_clear_filters();
  return true;
}

// FSM Transitions
static void prv_sequencer_state_emergency(FSM *fsm, const Event *e, void *context) {
  (void)fsm;
  (void)e;
  if (!prv_sequencer_setup_common()) {
    return;
  }
  prv_add_filter(CHAOS_EVENT_RELAY_ERROR);
  prv_add_filter(CHAOS_EVENT_RELAY_OPENED);
  event_raise(CHAOS_EVENT_SET_RELAY_RETRIES, RELAY_RETRY_SERVICE_UNLIMITED_ATTEMPTS);
  sequencer_init((SequencerStorage *)context, (SequencerEventPair *)s_emergency_events,
                 SIZEOF_ARRAY(s_emergency_events));
}

static void prv_sequencer_state_idle(FSM *fsm, const Event *e, void *context) {
  (void)fsm;
  (void)e;
  if (!prv_sequencer_setup_common()) {
    return;
  }
  event_raise(CHAOS_EVENT_SET_RELAY_RETRIES, RELAY_RETRY_SERVICE_DEFAULT_ATTEMPTS);
  sequencer_init((SequencerStorage *)context, (SequencerEventPair *)s_idle_events,
                 SIZEOF_ARRAY(s_idle_events));
}

static void prv_sequencer_state_charge(FSM *fsm, const Event *e, void *context) {
  (void)fsm;
  (void)e;
  if (!prv_sequencer_setup_common()) {
    return;
  }
  event_raise(CHAOS_EVENT_SET_RELAY_RETRIES, RELAY_RETRY_SERVICE_DEFAULT_ATTEMPTS);
  sequencer_init((SequencerStorage *)context, (SequencerEventPair *)s_charge_events,
                 SIZEOF_ARRAY(s_charge_events));
}

static void prv_sequencer_state_drive(FSM *fsm, const Event *e, void *context) {
  (void)fsm;
  (void)e;
  if (!prv_sequencer_setup_common()) {
    return;
  }
  event_raise(CHAOS_EVENT_SET_RELAY_RETRIES, RELAY_RETRY_SERVICE_DEFAULT_ATTEMPTS);
  sequencer_init((SequencerStorage *)context, (SequencerEventPair *)s_drive_events,
                 SIZEOF_ARRAY(s_drive_events));
}

// Public interface
void sequencer_fsm_init(void) {
  prv_clear_filters();
  s_pending_transition = false;
  memset(&s_storage, 0, sizeof(s_storage));
  fsm_state_init(sequencer_state_emergency, prv_sequencer_state_emergency);
  fsm_state_init(sequencer_state_idle, prv_sequencer_state_idle);
  fsm_state_init(sequencer_state_charge, prv_sequencer_state_charge);
  fsm_state_init(sequencer_state_drive, prv_sequencer_state_drive);
  fsm_init(&s_sequencer_fsm, "SequenceFSM", &sequencer_state_idle, &s_storage);
}

StatusCode sequencer_fsm_publish_next_event(const Event *previous_event) {
  // Try to transition if the event is a sequence transition.
  if (previous_event->id == CHAOS_EVENT_SEQUENCE_IDLE ||
      previous_event->id == CHAOS_EVENT_SEQUENCE_CHARGE ||
      previous_event->id == CHAOS_EVENT_SEQUENCE_DRIVE ||
      previous_event->id == CHAOS_EVENT_SEQUENCE_EMERGENCY ||
      previous_event->id == CHAOS_EVENT_SEQUENCE_RESET) {
    if (!fsm_process_event(&s_sequencer_fsm, previous_event)) {
      LOG_WARN("Failed transition: %u %u\n", previous_event->id, previous_event->data);
      return status_code(STATUS_CODE_INVALID_ARGS);
    }
    return STATUS_CODE_OK;
  }

  // Filter out both filtered messages. Also filter all events if there is a pending_transition.
  if (prv_is_filtered_id(previous_event->id)) {
    return STATUS_CODE_OK;
  }

  // Handle a totally faulted relay in a special way, ignore this kind of failure if we are
  // transitioning as the relay may not have reached the maximal retry limit.
  if (previous_event->id == CHAOS_EVENT_RELAY_ERROR) {
    if (s_pending_transition) {
      // Force the awaiting flag to clear as we the relay failed fast.
      s_storage.awaiting_response = false;
      return STATUS_CODE_OK;
    }
    // If we aren't in the emergency state we need to switch to that state. This event will not be
    // raised in the emergency state.
    return event_raise_priority(EVENT_PRIORITY_HIGH, CHAOS_EVENT_SEQUENCE_EMERGENCY,
                                SEQUENCER_EMPTY_DATA);
  }

  // Filter to only handled events. Those in the range
  // |(NUM_CHAOS_EVENTS_CAN, NUM_CHAOS_EVENTS)|
  if (previous_event->id <= NUM_CHAOS_EVENTS_CAN) {
    // Not in the range of handled events. This isn't necessarily invalid but won't be handled so
    // return unknown.
    return status_code(STATUS_CODE_UNKNOWN);
  }

  StatusCode status = sequencer_advance(&s_storage, previous_event);
  // An unexpected event occurred we should reset.
  if (status == STATUS_CODE_INTERNAL_ERROR) {
    if (s_retries < SEQUENCER_FSM_MAX_RETRIES) {
      s_retries++;
      return event_raise_priority(EVENT_PRIORITY_HIGH, CHAOS_EVENT_SEQUENCE_RESET,
                                  SEQUENCER_EMPTY_DATA);
    }
    // If we are stuck go to the emergency state.
    return event_raise_priority(EVENT_PRIORITY_HIGH, CHAOS_EVENT_SEQUENCE_EMERGENCY,
                                SEQUENCER_EMPTY_DATA);
  }
  return status;
}
