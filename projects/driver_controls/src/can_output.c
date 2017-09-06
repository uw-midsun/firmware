#include "can_output.h"
#include "event_arbiter.h"
#include "input_event.h"
#include "log.h"

// CAN device ids as defined in confluence
// https://uwmidsun.atlassian.net/wiki/spaces/ELEC/pages/50003973/CAN+Message+Definitions
#define CAN_OUTPUT_ID_POWER_STATE         5
#define CAN_OUTPUT_ID_MOTOR_THROTTLE      18
#define CAN_OUTPUT_ID_MOTOR_CRUISE        19
#define CAN_OUTPUT_ID_MOTOR_DIR_SELECT    20
#define CAN_OUTPUT_ID_LIGHTS              24
#define CAN_OUTPUT_ID_HORN                25

static uint8_t s_can_output_lookup[NUM_CAN_OUTPUT_MESSAGES] = {
  [CAN_OUTPUT_MESSAGE_POWER] = CAN_OUTPUT_ID_POWER_STATE,
  [CAN_OUTPUT_MESSAGE_PEDAL] = CAN_OUTPUT_ID_MOTOR_THROTTLE,
  [CAN_OUTPUT_MESSAGE_DIRECTION_SELECTOR] = CAN_OUTPUT_ID_MOTOR_DIR_SELECT,
  [CAN_OUTPUT_MESSAGE_TURN_SIGNAL] = CAN_OUTPUT_ID_LIGHTS,
  [CAN_OUTPUT_MESSAGE_HAZARD_LIGHT] = CAN_OUTPUT_ID_LIGHTS,
  [CAN_OUTPUT_MESSAGE_HORN] = CAN_OUTPUT_ID_HORN
};

void can_output_transmit(EventArbiterOutputData data) {
  LOG_DEBUG("Device = %d, State = %d, Data = %d\n",
             s_can_output_lookup[data.id],
             data.state,
             data.data);
}
