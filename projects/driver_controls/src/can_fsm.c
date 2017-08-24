#include "can_fsm.h"
#include "event_arbiter.h"
#include "input_event.h"
#include "log.h"

static FSM *s_can_fsm;

FSM_DECLARE_STATE(state_can_transmit);

FSM_STATE_TRANSITION(state_can_transmit) {
  FSM_ADD_TRANSITION(CAN_DEVICE_ID_POWER, state_can_transmit);
  FSM_ADD_TRANSITION(CAN_DEVICE_ID_PEDAL, state_can_transmit);
  FSM_ADD_TRANSITION(CAN_DEVICE_ID_DIRECTION_SELECTOR, state_can_transmit);
  FSM_ADD_TRANSITION(CAN_DEVICE_ID_TURN_SIGNAL, state_can_transmit);
  FSM_ADD_TRANSITION(CAN_DEVICE_ID_HAZARD_LIGHT, state_can_transmit);
  FSM_ADD_TRANSITION(CAN_DEVICE_ID_MECHANICAL_BRAKE, state_can_transmit);
  FSM_ADD_TRANSITION(CAN_DEVICE_ID_HORN, state_can_transmit);
  FSM_ADD_TRANSITION(CAN_DEVICE_ID_PUSH_TO_TALK, state_can_transmit);
}

static void prv_transmit_data(FSM *fsm, const Event *e, void *context) {
  printf("Device = %d, State = %d, Data = %d\n", e->id, e->data >> 12, e->data & 0xFFF);
}

StatusCode can_fsm_init(FSM *fsm) {
  s_can_fsm = fsm;

  fsm_state_init(state_can_transmit, prv_transmit_data);

  fsm_init(fsm, "can_fsm", &state_can_transmit, NULL);

  return STATUS_CODE_OK;
}

StatusCode can_fsm_transmit(CANDeviceID device_id, uint8_t device_state, uint16_t device_data) {
  union EventData {
    uint16_t raw;
    struct {
      uint16_t data : 12;
      uint8_t state : 4;
    } components;
  } data;

  data.components.data = device_data;
  data.components.state = device_state;

  Event e = {.id = device_id, .data = data.raw };
  fsm_process_event(s_can_fsm, &e);

  return STATUS_CODE_OK;
}
