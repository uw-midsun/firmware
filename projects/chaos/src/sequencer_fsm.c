#include "sequencer_fsm.h"

#include <stddef.h>
#include <string.h>

#include "chaos_events.h"
#include "critical_section.h"
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
static bool s_pending;
static ChaosEventSequence s_current_sequence;
static ChaosEventSequence s_next_sequence;

// Event order declarations

// Emergency State Order:
// - Turn off the charger (if active)
// - Disable DCDC Monitoring
// - Open main power relay
// - Open solar master rear relay
// - Open solar master front relay
// - Open battery relay
// - Disable all unnecessary GPIO pins
static const SequencerEventPair s_emergency_events[] = {
  { .raise = { .id = CHAOS_EVENT_CHARGER_OPEN, .data = SEQUENCER_EMPTY_DATA },
    .response = SEQUENCER_NO_RESPONSE },
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
// - Turn off the charger (if active)
// - Disable DCDC Monitoring
// - Open main power relay
// - Open solar master rear relay
// - Open solar master front relay
// - Open battery relay
// - Disable all unnecessary GPIO pins
static const SequencerEventPair s_idle_events[] = {
  { .raise = { .id = CHAOS_EVENT_CHARGER_OPEN, .data = SEQUENCER_EMPTY_DATA },
    .response = SEQUENCER_NO_RESPONSE },
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
// - Turn on the charger
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
  { .raise = { .id = CHAOS_EVENT_CHARGER_CLOSE, .data = SEQUENCER_EMPTY_DATA },
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
// NOTE: No need to manipulate the charger since you can't reach Drive from Charge.
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
FSM_DECLARE_STATE(sequencer_state_default);
FSM_DECLARE_STATE(sequencer_state_emergency);
FSM_DECLARE_STATE(sequencer_state_idle);
FSM_DECLARE_STATE(sequencer_state_charge);
FSM_DECLARE_STATE(sequencer_state_drive);

// Prior to the board booting begin in this state.
FSM_STATE_TRANSITION(sequencer_state_default) {
  FSM_ADD_TRANSITION(CHAOS_EVENT_SEQUENCE_IDLE, sequencer_state_idle);
}

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

static bool prv_is_valid_transition(ChaosEventSequence present, ChaosEventSequence next) {
  if (present == CHAOS_EVENT_SEQUENCE_IDLE) {
    return next == CHAOS_EVENT_SEQUENCE_CHARGE || next == CHAOS_EVENT_SEQUENCE_DRIVE ||
           next == CHAOS_EVENT_SEQUENCE_EMERGENCY;
  } else if (present == CHAOS_EVENT_SEQUENCE_CHARGE || present == CHAOS_EVENT_SEQUENCE_DRIVE ||
             present == CHAOS_EVENT_SEQUENCE_EMERGENCY) {
    return next == CHAOS_EVENT_SEQUENCE_IDLE || next == CHAOS_EVENT_SEQUENCE_EMERGENCY;
  }
  return false;
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
  fsm_state_init(sequencer_state_default, NULL);
  fsm_state_init(sequencer_state_emergency, prv_sequencer_state_emergency);
  fsm_state_init(sequencer_state_idle, prv_sequencer_state_idle);
  fsm_state_init(sequencer_state_charge, prv_sequencer_state_charge);
  fsm_state_init(sequencer_state_drive, prv_sequencer_state_drive);
  fsm_init(&s_sequencer_fsm, "SequenceFSM", &sequencer_state_default, &s_storage);
  s_next_sequence = NUM_CHAOS_EVENT_SEQUENCES;
  event_raise(CHAOS_EVENT_SEQUENCE_IDLE, SEQUENCER_EMPTY_DATA);
  s_pending = true;
  s_current_sequence = CHAOS_EVENT_SEQUENCE_IDLE;
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
    s_pending = false;
    return STATUS_CODE_OK;
  }

  bool crit = critical_section_start();
  status_ok_or_return(sequencer_advance(&s_storage, previous_event));

  // If the sequence is finished and there is a pending event then raise it.
  if (sequencer_complete(&s_storage) && s_next_sequence != NUM_CHAOS_EVENT_SEQUENCES) {
    event_raise(s_next_sequence, SEQUENCER_EMPTY_DATA);
    s_current_sequence = s_next_sequence;
    s_next_sequence = NUM_CHAOS_EVENT_SEQUENCES;
  }
  critical_section_end(crit);
  return STATUS_CODE_OK;
}

bool sequencer_fsm_event_raise(ChaosEventSequence sequence) {
  if (sequence >= NUM_CHAOS_EVENT_SEQUENCES || sequence < CHAOS_EVENT_SEQUENCE_EMERGENCY) {
    return false;
  }
  bool ret = false;
  bool crit = critical_section_start();
  if (prv_is_valid_transition(s_current_sequence, sequence)) {
    if (sequencer_complete(&s_storage) && !s_pending) {
      // The sequence is already finished so just raise the sequence.
      event_raise(sequence, SEQUENCER_EMPTY_DATA);
      s_current_sequence = sequence;
      s_pending = true;
      ret = true;
    } else {
      // Sequence is in progress. If the new sequence is higher (or the same sequence as the queued
      // one) then update the |s_next_sequence| with the next expected sequence.
      if (s_next_sequence >= sequence) {
        s_next_sequence = sequence;
        ret = true;
      }
    }
  }
  critical_section_end(crit);
  return ret;
}
