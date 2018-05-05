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
#include "sequencer.h"

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
  { .raise = { .id = CHAOS_EVENT_MONITOR_DISABLE, .data = POWER_PATH_SOURCE_ID_DCDC },
    .response = SEQUENCER_NO_RESPONSE },
  { .raise = { .id = CHAOS_EVENT_CLOSE_RELAY, .data = RELAY_ID_MAIN_POWER },
    .response = { .id = CHAOS_EVENT_RELAY_CLOSED, .data = RELAY_ID_MAIN_POWER } },
  { .raise = { .id = CHAOS_EVENT_CLOSE_RELAY, .data = RELAY_ID_SOLAR_MASTER_REAR },
    .response = { .id = CHAOS_EVENT_RELAY_CLOSED, .data = RELAY_ID_SOLAR_MASTER_REAR } },
  { .raise = { .id = CHAOS_EVENT_CLOSE_RELAY, .data = RELAY_ID_SOLAR_MASTER_FRONT },
    .response = { .id = CHAOS_EVENT_RELAY_CLOSED, .data = RELAY_ID_SOLAR_MASTER_FRONT } },
  { .raise = { .id = CHAOS_EVENT_CLOSE_RELAY, .data = RELAY_ID_BATTERY },
    .response = { .id = CHAOS_EVENT_RELAY_CLOSED, .data = RELAY_ID_BATTERY } },
  { .raise = { .id = CHAOS_EVENT_GPIO_EMERGENCY, .data = SEQUENCER_EMPTY_DATA },
    .response = SEQUENCER_NO_RESPONSE },
  { .raise = { .id = CHAOS_EVENT_SEQUENCE_EMERGENCY_DONE, .data = SEQUENCER_EMPTY_DATA },
    .response = SEQUENCER_NO_RESPONSE },
};

// Idle State Order:
// - Disable DCDC Monitoring
// - Open main power relay
// - Open solar master rear relay
// - Open solar master front relay
// - Open battery relay
// - Disable all unnecessary GPIO pins
static const SequencerEventPair s_idle_events[] = {
  { .raise = { .id = CHAOS_EVENT_MONITOR_DISABLE, .data = POWER_PATH_SOURCE_ID_DCDC },
    .response = SEQUENCER_NO_RESPONSE },
  { .raise = { .id = CHAOS_EVENT_CLOSE_RELAY, .data = RELAY_ID_MAIN_POWER },
    .response = { .id = CHAOS_EVENT_RELAY_CLOSED, .data = RELAY_ID_MAIN_POWER } },
  { .raise = { .id = CHAOS_EVENT_CLOSE_RELAY, .data = RELAY_ID_SOLAR_MASTER_REAR },
    .response = { .id = CHAOS_EVENT_RELAY_CLOSED, .data = RELAY_ID_SOLAR_MASTER_REAR } },
  { .raise = { .id = CHAOS_EVENT_CLOSE_RELAY, .data = RELAY_ID_SOLAR_MASTER_FRONT },
    .response = { .id = CHAOS_EVENT_RELAY_CLOSED, .data = RELAY_ID_SOLAR_MASTER_FRONT } },
  { .raise = { .id = CHAOS_EVENT_CLOSE_RELAY, .data = RELAY_ID_BATTERY },
    .response = { .id = CHAOS_EVENT_RELAY_CLOSED, .data = RELAY_ID_BATTERY } },
  { .raise = { .id = CHAOS_EVENT_GPIO_IDLE, .data = SEQUENCER_EMPTY_DATA },
    .response = SEQUENCER_NO_RESPONSE },
  { .raise = { .id = CHAOS_EVENT_SEQUENCE_IDLE_DONE, .data = SEQUENCER_EMPTY_DATA },
    .response = SEQUENCER_NO_RESPONSE },
};

// Charge State Order:
// - Turn on charging GPIOs
// - Open battery relay
// - Open solar master rear relay
// - Open solar master front relay
// - Open main power relay
// - Enable DCDC monitoring
static const SequencerEventPair s_charge_events[] = {
  { .raise = { .id = CHAOS_EVENT_GPIO_CHARGE, .data = SEQUENCER_EMPTY_DATA },
    .response = SEQUENCER_NO_RESPONSE },
  { .raise = { .id = CHAOS_EVENT_CLOSE_RELAY, .data = RELAY_ID_BATTERY },
    .response = { .id = CHAOS_EVENT_RELAY_CLOSED, .data = RELAY_ID_BATTERY } },
  { .raise = { .id = CHAOS_EVENT_CLOSE_RELAY, .data = RELAY_ID_SOLAR_MASTER_REAR },
    .response = { .id = CHAOS_EVENT_RELAY_CLOSED, .data = RELAY_ID_SOLAR_MASTER_REAR } },
  { .raise = { .id = CHAOS_EVENT_CLOSE_RELAY, .data = RELAY_ID_SOLAR_MASTER_FRONT },
    .response = { .id = CHAOS_EVENT_RELAY_CLOSED, .data = RELAY_ID_SOLAR_MASTER_FRONT } },
  { .raise = { .id = CHAOS_EVENT_CLOSE_RELAY, .data = RELAY_ID_MAIN_POWER },
    .response = { .id = CHAOS_EVENT_RELAY_CLOSED, .data = RELAY_ID_MAIN_POWER } },
  { .raise = { .id = CHAOS_EVENT_MONITOR_ENABLE, .data = POWER_PATH_SOURCE_ID_DCDC },
    .response = SEQUENCER_NO_RESPONSE },
  { .raise = { .id = CHAOS_EVENT_SEQUENCE_CHARGE_DONE, .data = SEQUENCER_EMPTY_DATA },
    .response = SEQUENCER_NO_RESPONSE },
};

// Drive State Order:
// - Turn on drive GPIOs
// - Open battery relay
// - Open solar master rear relay
// - Open solar master front relay
// - Open main power relay
// - Enable DCDC monitoring
static const SequencerEventPair s_drive_events[] = {
  { .raise = { .id = CHAOS_EVENT_GPIO_DRIVE, .data = SEQUENCER_EMPTY_DATA },
    .response = SEQUENCER_NO_RESPONSE },
  { .raise = { .id = CHAOS_EVENT_CLOSE_RELAY, .data = RELAY_ID_BATTERY },
    .response = { .id = CHAOS_EVENT_RELAY_CLOSED, .data = RELAY_ID_BATTERY } },
  { .raise = { .id = CHAOS_EVENT_CLOSE_RELAY, .data = RELAY_ID_SOLAR_MASTER_REAR },
    .response = { .id = CHAOS_EVENT_RELAY_CLOSED, .data = RELAY_ID_SOLAR_MASTER_REAR } },
  { .raise = { .id = CHAOS_EVENT_CLOSE_RELAY, .data = RELAY_ID_SOLAR_MASTER_FRONT },
    .response = { .id = CHAOS_EVENT_RELAY_CLOSED, .data = RELAY_ID_SOLAR_MASTER_FRONT } },
  { .raise = { .id = CHAOS_EVENT_CLOSE_RELAY, .data = RELAY_ID_MAIN_POWER },
    .response = { .id = CHAOS_EVENT_RELAY_CLOSED, .data = RELAY_ID_MAIN_POWER } },
  { .raise = { .id = CHAOS_EVENT_MONITOR_ENABLE, .data = POWER_PATH_SOURCE_ID_DCDC },
    .response = SEQUENCER_NO_RESPONSE },
  { .raise = { .id = CHAOS_EVENT_SEQUENCE_DRIVE_DONE, .data = SEQUENCER_EMPTY_DATA },
    .response = SEQUENCER_NO_RESPONSE },
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
  sequencer_init((SequencerStorage *)context, (SequencerEventPair *)s_emergency_events,
                 SIZEOF_ARRAY(s_emergency_events));
}

static void prv_sequencer_state_idle(FSM *fsm, const Event *e, void *context) {
  (void)fsm;
  (void)e;
  sequencer_init((SequencerStorage *)context, (SequencerEventPair *)s_idle_events,
                 SIZEOF_ARRAY(s_idle_events));
}

static void prv_sequencer_state_charge(FSM *fsm, const Event *e, void *context) {
  (void)fsm;
  (void)e;
  sequencer_init((SequencerStorage *)context, (SequencerEventPair *)s_charge_events,
                 SIZEOF_ARRAY(s_charge_events));
}

static void prv_sequencer_state_drive(FSM *fsm, const Event *e, void *context) {
  (void)fsm;
  (void)e;
  sequencer_init((SequencerStorage *)context, (SequencerEventPair *)s_drive_events,
                 SIZEOF_ARRAY(s_drive_events));
}

// Public interface
void sequencer_fsm_init(void) {
  memset(&s_storage, 0, sizeof(s_storage));
  fsm_state_init(sequencer_state_emergency, prv_sequencer_state_emergency);
  fsm_state_init(sequencer_state_idle, prv_sequencer_state_idle);
  fsm_state_init(sequencer_state_charge, prv_sequencer_state_charge);
  fsm_state_init(sequencer_state_drive, prv_sequencer_state_drive);
  fsm_init(&s_sequencer_fsm, "SequenceFSM", &sequencer_state_idle, &s_storage);
}

StatusCode sequencer_fsm_publish_next_event(const Event *previous_event) {
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

  return sequencer_advance(&s_storage, previous_event);
}
