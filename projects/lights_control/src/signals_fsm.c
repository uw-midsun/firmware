#include "status.h"
#include "fsm.h"
#include "string.h"

#include "lights_events.h"
#include "lights_gpio.h"
#include "blinker.h"
#include "signals_fsm.h"

FSM_DECLARE_STATE(state_none);
FSM_DECLARE_STATE(state_signal_left);
FSM_DECLARE_STATE(state_signal_right);
FSM_DECLARE_STATE(state_hazard);

static bool prv_data_on(const FSM *fsm, const Event *e, void *context) {
  return (bool)e->data;
}

static bool prv_data_off(const FSM *fsm, const Event *e, void *context) {
  return (bool)e->data;
}

static bool prv_left_previously(const FSM *fsm, const Event *e, void *context) {
  char *last_state = fsm->last_state->name;
  return strcmp("state_signal_left", last_state) == 0 && !e->data;
}

static bool prv_right_previously(const FSM *fsm, const Event *e, void *context) {
  char *last_state = fsm->last_state->name;
  return strcmp("state_signal_right", last_state) == 0 && !e->data;
}

static bool prv_none_previously(const FSM *fsm, const Event *e, void *context) {
  char *last_state = fsm->last_state->name;
  return strcmp("state_none", last_state) == 0 && !e->data;
}

FSM_STATE_TRANSITION(state_none) {
  FSM_ADD_GUARDED_TRANSITION(EVENT_SIGNAL_LEFT, prv_data_on, state_signal_left);
  FSM_ADD_GUARDED_TRANSITION(EVENT_SIGNAL_RIGHT, prv_data_on, state_signal_right);
  FSM_ADD_GUARDED_TRANSITION(EVENT_SIGNAL_HAZARD, prv_data_on, state_hazard);
}

FSM_STATE_TRANSITION(state_signal_left) {
  FSM_ADD_GUARDED_TRANSITION(EVENT_SIGNAL_LEFT, prv_data_off, state_none);
  FSM_ADD_GUARDED_TRANSITION(EVENT_SIGNAL_HAZARD, prv_data_on, state_hazard);
}

FSM_STATE_TRANSITION(state_signal_right) {
  FSM_ADD_GUARDED_TRANSITION(EVENT_SIGNAL_RIGHT, prv_data_off, state_none);
  FSM_ADD_GUARDED_TRANSITION(EVENT_SIGNAL_HAZARD, prv_data_on, state_hazard);
}

FSM_STATE_TRANSITION(state_hazard) {
  FSM_ADD_GUARDED_TRANSITION(EVENT_SIGNAL_HAZARD, prv_left_previously, state_signal_left);
  FSM_ADD_GUARDED_TRANSITION(EVENT_SIGNAL_HAZARD, prv_right_previously, state_signal_right);
  FSM_ADD_GUARDED_TRANSITION(EVENT_SIGNAL_HAZARD, prv_none_previously, state_none);
}

Blinker s_blinker_left;

Blinker s_blinker_right;

static BlinkerDuration s_duration_millis;

static SignalCallback prv_signal_callback;

static SignalSyncCallback prv_signal_sync_callback;

static void prv_left_blinker_cb(BlinkerState s) {
  Event e = {
    .id = EVENT_SIGNAL_LEFT,
    .data = (uint16_t) s
  }
  prv_signal_callback(e);
}

static void prv_right_blinker_cb(BlinkerState s) {
  Event e = {
    .id = EVENT_SIGNAL_RIGHT,
    .data = (uint16_t) s
  }
  prv_signal_callback(e);
}

static void prv_blinker_sync_cb(void) {
  prv_signal_sync_callback();
}

static BoardType s_boardtype;

StatusCode signals_fsm_init(FSM *fsm, BoardType boardtype, 
                SignalCallback cb, BlinkerDuration duration,
                SignalSyncCallback sync_cb,
                uint8_t sync_frequency) {
  s_duration_millis = duration;
  s_boardtype = boardtype;
  prv_signal_callback = cb;
  prv_signal_sync_callback = sync_cb;
  if (boardtype == LIGHTS_BOARD_FRONT) {
    blinker_init(&s_blinker_left, prv_left_blinker_cb);
    blinker_init(&s_blinker_right, prv_right_blinker_cb);
  } else {
    blinker_init_sync(&s_blinker_left, prv_left_blinker_cb,
                    prv_blinker_sync_cb, sync_frequency);
    blinker_init_sync(&s_blinker_right, prv_right_blinker_cb,
                    prv_blinker_sync_cb, sync_frequency);
  }

  return STATUS_CODE_OK;
}

