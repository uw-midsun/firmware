/*
#include <stddef.h>
#include "fsm.h"

FSM_DECLARE_STATE(led_on);
FSM_DECLARE_STATE(led_off);

FSM_STATE_TRANSITION(led_off) {
	FSM_ADD_TRANSITION(CAN_LED_EVENT_TURN_ON, led_on);
}

FSM_STATE_TRANSITION(led_on) {
	FSM_ADD_TRANSITION(CAN_LED_EVENT_TURN_OFF, led_off); 
}

static void prv_state_led_on(Fsm *fsm, const Event *e, void *context) {
  (void)fsm;
  (void)e;
  (void)context;
  // No need for requests since the device is disconnected. Don't request charging.
  //charger_controller_set_state(CHARGER_STATE_STOP);
  //notify_cease();
}

static void prv_state_led_off(Fsm *fsm, const Event *e, void *context) {
  (void)fsm;
  (void)e;
  (void)context;
  // Request charging but don't start the charger.
  //charger_controller_set_state(CHARGER_STATE_STOP);
  //notify_post();
}
*/