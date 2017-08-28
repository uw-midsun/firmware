#include "can_output.h"
#include "event_arbiter.h"
#include "input_event.h"
#include "log.h"

void can_output_transmit(FSM *fsm, EventArbiterOutputData data) {
  printf("Device = %d, State = %d, Data = %d\n",
          data.id,
          data.state,
          data.data);
}
