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
static uint16_t s_event_idx;
static uint16_t s_sizeof_events = 0;

FSM_DECLARE_STATE(idle);
FSM_DECLARE_STATE(charge);
FSM_DECLARE_STATE(drive);

FSM_STATE_TRANSITION(idle) {
  FSM_ADD_TRANSITION(CHAOS_EVENT_SEQUENCE_RESET, idle);
  FSM_ADD_TRANSITION(CHAOS_EVENT_SEQUENCE_CHARGE, charge);
  FSM_ADD_TRANSITION(CHAOS_EVENT_SEQUENCE_DRIVE, drive);
}

FSM_STATE_TRANSITION(charge) {
  FSM_ADD_TRANSITION(CHAOS_EVENT_SEQUENCE_IDLE, idle);
  FSM_ADD_TRANSITION(CHAOS_EVENT_SEQUENCE_RESET, charge);
}

FSM_STATE_TRANSITION(drive) {
  FSM_ADD_TRANSITION(CHAOS_EVENT_SEQUENCE_IDLE, idle);
  FSM_ADD_TRANSITION(CHAOS_EVENT_SEQUENCE_RESET, drive);
}

static void prv_state_idle(FSM *fsm, const Event *e, void *context) {
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
  };
  s_event_idx = 0;
  s_sizeof_events = SIZEOF_ARRAY(idle_events);
  s_events = idle_events;
}

static void prv_state_charge(FSM *fsm, const Event *e, void *context) {
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
  };
  s_event_idx = 0;
  s_sizeof_events = SIZEOF_ARRAY(charge_events);
  s_events = charge_events;
}

static void prv_state_drive(FSM *fsm, const Event *e, void *context) {
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
  };
  s_event_idx = 0;
  s_sizeof_events = SIZEOF_ARRAY(drive_events);
  s_events = drive_events;
}

void sequencer_init(void) {
  fsm_state_init(idle, prv_state_idle);
  fsm_state_init(charge, prv_state_charge);
  fsm_state_init(drive, prv_state_drive);
  fsm_init(&s_sequencer_fsm, "SequenceFSM", &idle, NULL);
  // Invalid event_idx since vehicle starts IDLE.
  s_event_idx = UINT16_MAX;
}

StatusCode sequencer_publish_next_event(const Event *previous_event) {
  if (previous_event->id >= NUM_CHAOS_EVENT_SEQUENCES ||
      previous_event->id <= NUM_CHAOS_EVENTS_CAN) {
    // Not in the range of handled events. This isn't necessarily invalid but won't be handled so
    // return unknown.
    return STATUS_CODE_UNKNOWN;
  } else if (previous_event->id == CHAOS_EVENT_NO_OP) {
    return STATUS_CODE_OK;
  }

  // Try to transition
  if ((previous_event->id > NUM_CHAOS_EVENTS_FSM) &&
      (previous_event->id < NUM_CHAOS_EVENT_SEQUENCES)) {
    if (!fsm_process_event(&s_sequencer_fsm, previous_event)) {
      LOG_DEBUG("Err: %u %u\n", previous_event->id, previous_event->data);
      return status_code(STATUS_CODE_INVALID_ARGS);
    }
  }

  // Stop if there are no more events.
  if (s_event_idx >= s_sizeof_events) {
    return status_code(STATUS_CODE_RESOURCE_EXHAUSTED);
  }

  // If the s_event_idx == 0 we have no need to check the previous event.
  if (s_event_idx == 0) {
    event_raise(s_events[s_event_idx].id, s_events[s_event_idx].data);
    s_event_idx++;
    return STATUS_CODE_OK;
  }

  // Check that the previous event was expected or restart the sequence otherwise.
  if (memcmp(previous_event, &s_events[s_event_idx - 1], sizeof(Event)) != 0) {
    event_raise(CHAOS_EVENT_SEQUENCE_RESET, 0);
    return status_msg(STATUS_CODE_INTERNAL_ERROR, "Restarting Sequence");
  }

  Event event = s_events[s_event_idx];
  if (event.id == CHAOS_EVENT_NO_OP) {
    // Skip the event we have to wait on an external event.
    s_event_idx += 2;
  } else {
    // Raise the next event to execute.
    event_raise(event.id, event.data);
    s_event_idx++;
  }
  return STATUS_CODE_OK;
}
