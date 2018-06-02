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

#define NUM_SEQUENCER_FSM_FILTERS 3

// Statics
static FSM s_sequencer_fsm;
static SequencerStorage s_storage;

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

// Emergency State Order:
// - Turn off the charger (if active)
// - Disable all unnecessary GPIO pins (this will auto-open all the non-battery relays)
// - Open main power relay (no-op to sync FSM)
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
// - Open main power relay
// - Open solar master rear relay
// - Open solar master front relay
// - Disable all unnecessary GPIO pins
// - Disable DCDC Monitoring
// - Open battery relay (DCDC)
static const SequencerEventPair s_idle_events[] = {
  { .raise = { .id = CHAOS_EVENT_CHARGER_OPEN, .data = SEQUENCER_EMPTY_DATA },
    .response = SEQUENCER_NO_RESPONSE },
  { .raise = { .id = CHAOS_EVENT_OPEN_RELAY, .data = RELAY_ID_MOTORS },
    .response = { .id = CHAOS_EVENT_RELAY_OPENED, .data = RELAY_ID_MOTORS } },
  { .raise = { .id = CHAOS_EVENT_OPEN_RELAY, .data = RELAY_ID_SOLAR_MASTER_REAR },
    .response = { .id = CHAOS_EVENT_RELAY_OPENED, .data = RELAY_ID_SOLAR_MASTER_REAR } },
  { .raise = { .id = CHAOS_EVENT_OPEN_RELAY, .data = RELAY_ID_SOLAR_MASTER_FRONT },
    .response = { .id = CHAOS_EVENT_RELAY_OPENED, .data = RELAY_ID_SOLAR_MASTER_FRONT } },
  { .raise = { .id = CHAOS_EVENT_GPIO_IDLE, .data = SEQUENCER_EMPTY_DATA },
    .response = SEQUENCER_NO_RESPONSE },
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
// - Close battery relay (DCDC)
// - Enable DCDC monitoring
// - Turn on charging GPIOs
// - Close solar master rear relay
// - Close solar master front relay
// - Turn on the charger
// NOTE: No need to manipulate the main relay since it turns on the motors and will be open by
// default before reaching this state.
static const SequencerEventPair s_charge_events[] = {
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
// - Close battery relay (DCDC)
// - Enable DCDC monitoring
// - Turn on drive GPIOs
// - Close solar master rear relay
// - Close solar master front relay
// - Close main power relay
// NOTE: No need to manipulate the charger since you can't reach Drive from Charge.
static const SequencerEventPair s_drive_events[] = {
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
  if (s_storage.awaiting_response) {
    relay_retry_service_fail_fast();
    s_pending_transition = true;
    // We actually transitioned so just reset the state until it succeeds.
    event_raise(CHAOS_EVENT_SEQUENCE_RESET, SEQUENCER_EMPTY_DATA);
    return false;
  }
  s_pending_transition = false;
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
  sequencer_init((SequencerStorage *)context, (SequencerEventPair *)s_emergency_events,
                 SIZEOF_ARRAY(s_emergency_events));
}

static void prv_sequencer_state_idle(FSM *fsm, const Event *e, void *context) {
  (void)fsm;
  (void)e;
  if (!prv_sequencer_setup_common()) {
    return;
  }
  sequencer_init((SequencerStorage *)context, (SequencerEventPair *)s_idle_events,
                 SIZEOF_ARRAY(s_idle_events));
}

static void prv_sequencer_state_charge(FSM *fsm, const Event *e, void *context) {
  (void)fsm;
  (void)e;
  if (!prv_sequencer_setup_common()) {
    return;
  }
  sequencer_init((SequencerStorage *)context, (SequencerEventPair *)s_charge_events,
                 SIZEOF_ARRAY(s_charge_events));
}

static void prv_sequencer_state_drive(FSM *fsm, const Event *e, void *context) {
  (void)fsm;
  (void)e;
  if (!prv_sequencer_setup_common()) {
    return;
  }
  sequencer_init((SequencerStorage *)context, (SequencerEventPair *)s_drive_events,
                 SIZEOF_ARRAY(s_drive_events));
}

// Public interface
void sequencer_fsm_init(void) {
  prv_clear_filters();
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
  if (prv_is_filtered_id(previous_event->id) || s_pending_transition) {
    return STATUS_CODE_OK;
  }

  // Filter to only handled events. Those in the range
  // |(NUM_CHAOS_EVENTS_CAN, NUM_CHAOS_EVENTS_FSM)|
  // in addition to the emergency sequence messages.
  if (previous_event->id <= NUM_CHAOS_EVENTS_CAN &&
      !(previous_event->id == CHAOS_EVENT_SEQUENCE_EMERGENCY ||
        previous_event->id == CHAOS_EVENT_SEQUENCE_EMERGENCY_DONE)) {
    // Not in the range of handled events. This isn't necessarily invalid but won't be handled so
    // return unknown.
    return status_code(STATUS_CODE_UNKNOWN);
  }

  return sequencer_advance(&s_storage, previous_event);
}
