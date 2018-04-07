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

#define EMPTY_DATA 0

static FSM s_sequencer_fsm;
static const Event *s_events;
static uint8_t s_retries = 0;
static uint16_t s_event_idx;
static uint16_t s_sizeof_events = 0;

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

static void prv_sequencer_state_emergency(FSM *fsm, const Event *e, void *context) {
  (void)fsm;
  (void)e;
  (void)context;
  static const Event emergency_events[] = {
    { .id = CHAOS_EVENT_MONITOR_DISABLE, .data = POWER_PATH_SOURCE_ID_DCDC },
    { .id = CHAOS_EVENT_OPEN_RELAY, .data = RELAY_ID_MAIN_POWER },
    { .id = CHAOS_EVENT_NO_OP, .data = EMPTY_DATA },
    { .id = CHAOS_EVENT_RELAY_OPENED, .data = RELAY_ID_MAIN_POWER },
    { .id = CHAOS_EVENT_OPEN_RELAY, .data = RELAY_ID_SOLAR_MASTER_REAR },
    { .id = CHAOS_EVENT_NO_OP, .data = EMPTY_DATA },
    { .id = CHAOS_EVENT_RELAY_OPENED, .data = RELAY_ID_SOLAR_MASTER_REAR },
    { .id = CHAOS_EVENT_OPEN_RELAY, .data = RELAY_ID_SOLAR_MASTER_FRONT },
    { .id = CHAOS_EVENT_NO_OP, .data = EMPTY_DATA },
    { .id = CHAOS_EVENT_RELAY_OPENED, .data = RELAY_ID_SOLAR_MASTER_FRONT },
    { .id = CHAOS_EVENT_OPEN_RELAY, .data = RELAY_ID_BATTERY },
    { .id = CHAOS_EVENT_NO_OP, .data = EMPTY_DATA },
    { .id = CHAOS_EVENT_RELAY_OPENED, .data = RELAY_ID_BATTERY },
    { .id = CHAOS_EVENT_GPIO_EMERGENCY, .data = EMPTY_DATA },
    { .id = CHAOS_EVENT_SEQUENCE_EMERGENCY_DONE, .data = EMPTY_DATA },
  };
  s_event_idx = 0;
  s_sizeof_events = SIZEOF_ARRAY(emergency_events);
  s_events = emergency_events;
}

static void prv_sequencer_state_idle(FSM *fsm, const Event *e, void *context) {
  (void)fsm;
  (void)e;
  (void)context;
  static const Event idle_events[] = {
    { .id = CHAOS_EVENT_MONITOR_DISABLE, .data = POWER_PATH_SOURCE_ID_DCDC },
    { .id = CHAOS_EVENT_OPEN_RELAY, .data = RELAY_ID_MAIN_POWER },
    { .id = CHAOS_EVENT_NO_OP, .data = EMPTY_DATA },
    { .id = CHAOS_EVENT_RELAY_OPENED, .data = RELAY_ID_MAIN_POWER },
    { .id = CHAOS_EVENT_OPEN_RELAY, .data = RELAY_ID_SOLAR_MASTER_REAR },
    { .id = CHAOS_EVENT_NO_OP, .data = EMPTY_DATA },
    { .id = CHAOS_EVENT_RELAY_OPENED, .data = RELAY_ID_SOLAR_MASTER_REAR },
    { .id = CHAOS_EVENT_OPEN_RELAY, .data = RELAY_ID_SOLAR_MASTER_FRONT },
    { .id = CHAOS_EVENT_NO_OP, .data = EMPTY_DATA },
    { .id = CHAOS_EVENT_RELAY_OPENED, .data = RELAY_ID_SOLAR_MASTER_FRONT },
    { .id = CHAOS_EVENT_OPEN_RELAY, .data = RELAY_ID_BATTERY },
    { .id = CHAOS_EVENT_NO_OP, .data = EMPTY_DATA },
    { .id = CHAOS_EVENT_RELAY_OPENED, .data = RELAY_ID_BATTERY },
    { .id = CHAOS_EVENT_GPIO_IDLE, .data = EMPTY_DATA },
    { .id = CHAOS_EVENT_SEQUENCE_IDLE_DONE, .data = EMPTY_DATA },
  };
  s_event_idx = 0;
  s_sizeof_events = SIZEOF_ARRAY(idle_events);
  s_events = idle_events;
}

static void prv_sequencer_state_charge(FSM *fsm, const Event *e, void *context) {
  (void)fsm;
  (void)e;
  (void)context;
  static const Event charge_events[] = {
    { .id = CHAOS_EVENT_GPIO_CHARGE, .data = EMPTY_DATA },
    { .id = CHAOS_EVENT_CLOSE_RELAY, .data = RELAY_ID_BATTERY },
    { .id = CHAOS_EVENT_NO_OP, .data = EMPTY_DATA },
    { .id = CHAOS_EVENT_RELAY_CLOSED, .data = RELAY_ID_BATTERY },
    { .id = CHAOS_EVENT_CLOSE_RELAY, .data = RELAY_ID_SOLAR_MASTER_REAR },
    { .id = CHAOS_EVENT_NO_OP, .data = EMPTY_DATA },
    { .id = CHAOS_EVENT_RELAY_CLOSED, .data = RELAY_ID_SOLAR_MASTER_REAR },
    { .id = CHAOS_EVENT_CLOSE_RELAY, .data = RELAY_ID_SOLAR_MASTER_FRONT },
    { .id = CHAOS_EVENT_NO_OP, .data = EMPTY_DATA },
    { .id = CHAOS_EVENT_RELAY_CLOSED, .data = RELAY_ID_SOLAR_MASTER_FRONT },
    { .id = CHAOS_EVENT_CLOSE_RELAY, .data = RELAY_ID_MAIN_POWER },
    { .id = CHAOS_EVENT_NO_OP, .data = EMPTY_DATA },
    { .id = CHAOS_EVENT_RELAY_CLOSED, .data = RELAY_ID_MAIN_POWER },
    { .id = CHAOS_EVENT_MONITOR_ENABLE, .data = POWER_PATH_SOURCE_ID_DCDC },
    { .id = CHAOS_EVENT_SEQUENCE_CHARGE_DONE, .data = EMPTY_DATA },
  };
  s_event_idx = 0;
  s_sizeof_events = SIZEOF_ARRAY(charge_events);
  s_events = charge_events;
}

static void prv_sequencer_state_drive(FSM *fsm, const Event *e, void *context) {
  (void)fsm;
  (void)e;
  (void)context;
  static const Event drive_events[] = {
    { .id = CHAOS_EVENT_GPIO_DRIVE, .data = EMPTY_DATA },
    { .id = CHAOS_EVENT_CLOSE_RELAY, .data = RELAY_ID_BATTERY },
    { .id = CHAOS_EVENT_NO_OP, .data = EMPTY_DATA },
    { .id = CHAOS_EVENT_RELAY_CLOSED, .data = RELAY_ID_BATTERY },
    { .id = CHAOS_EVENT_CLOSE_RELAY, .data = RELAY_ID_SOLAR_MASTER_REAR },
    { .id = CHAOS_EVENT_NO_OP, .data = EMPTY_DATA },
    { .id = CHAOS_EVENT_RELAY_CLOSED, .data = RELAY_ID_SOLAR_MASTER_REAR },
    { .id = CHAOS_EVENT_CLOSE_RELAY, .data = RELAY_ID_SOLAR_MASTER_FRONT },
    { .id = CHAOS_EVENT_NO_OP, .data = EMPTY_DATA },
    { .id = CHAOS_EVENT_RELAY_CLOSED, .data = RELAY_ID_SOLAR_MASTER_FRONT },
    { .id = CHAOS_EVENT_CLOSE_RELAY, .data = RELAY_ID_MAIN_POWER },
    { .id = CHAOS_EVENT_NO_OP, .data = EMPTY_DATA },
    { .id = CHAOS_EVENT_RELAY_CLOSED, .data = RELAY_ID_MAIN_POWER },
    { .id = CHAOS_EVENT_MONITOR_ENABLE, .data = POWER_PATH_SOURCE_ID_DCDC },
    { .id = CHAOS_EVENT_SEQUENCE_DRIVE_DONE, .data = EMPTY_DATA },
  };
  s_event_idx = 0;
  s_sizeof_events = SIZEOF_ARRAY(drive_events);
  s_events = drive_events;
}

void sequencer_init(void) {
  s_retries = 0;
  fsm_state_init(sequencer_state_emergency, prv_sequencer_state_emergency);
  fsm_state_init(sequencer_state_idle, prv_sequencer_state_idle);
  fsm_state_init(sequencer_state_charge, prv_sequencer_state_charge);
  fsm_state_init(sequencer_state_drive, prv_sequencer_state_drive);
  fsm_init(&s_sequencer_fsm, "SequenceFSM", &sequencer_state_idle, NULL);
  // Invalid event_idx since vehicle starts IDLE.
  s_event_idx = UINT16_MAX;
}

StatusCode sequencer_publish_next_event(const Event *previous_event) {
  if (previous_event->id <= NUM_CHAOS_EVENTS_CAN &&
      !(previous_event->id == CHAOS_EVENT_SEQUENCE_EMERGENCY ||
        previous_event->id == CHAOS_EVENT_SEQUENCE_EMERGENCY_DONE)) {
    // Not in the range of handled events. This isn't necessarily invalid but won't be handled so
    // return unknown.
    return STATUS_CODE_UNKNOWN;
  } else if (previous_event->id == CHAOS_EVENT_NO_OP) {
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
  if (s_event_idx >= s_sizeof_events) {
    s_retries = 0;
    return status_code(STATUS_CODE_RESOURCE_EXHAUSTED);
  }

  // If the s_event_idx == 0 we have no need to check the previous event.
  if (s_event_idx == 0) {
    event_raise(s_events[s_event_idx].id, s_events[s_event_idx].data);
    s_event_idx++;
    return STATUS_CODE_OK;
  }

  // Check that the previous event was expected or restart the sequence otherwise.
  // NOTE: this implicitly means any events in the range
  // (NUM_CHAOS_EVENTS_CAN, NUM_CHAOS_EVENTS_FSM) must originate from the sequencer!!! This is to
  // ensure redundancy and allow retry attempts. If this fails 3 times consecutively then a serious
  // fault has occurred with Chaos. We enter the emergency state as something terrible has happened.
  // Realistically this is almost impossible and would only be triggered by relays repeatedly
  // failing to transition.
  if (memcmp(previous_event, &s_events[s_event_idx - 1], sizeof(Event)) != 0) {
    s_retries++;
    if (s_retries >= 3) {
      event_raise(CHAOS_EVENT_SEQUENCE_EMERGENCY, 0);
      return status_msg(STATUS_CODE_INTERNAL_ERROR, "Consistent failure! Emergency");
    }
    event_raise(CHAOS_EVENT_SEQUENCE_RESET, 0);
    return status_msg(STATUS_CODE_INTERNAL_ERROR, "Restarting Sequence");
  }

  Event event = s_events[s_event_idx];
  if (event.id == CHAOS_EVENT_NO_OP) {
    // Skip the event we have to wait on an external event. This works by skipping the NO_OP and the
    // expected event in the list. Then when processing the next event we check the expected event.
    s_event_idx += 2;
  } else {
    // Raise the next event to execute.
    event_raise(event.id, event.data);
    s_event_idx++;
  }
  return STATUS_CODE_OK;
}
