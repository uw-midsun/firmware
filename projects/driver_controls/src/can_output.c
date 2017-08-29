#include "can_output.h"
#include "event_arbiter.h"
#include "input_event.h"
#include "log.h"

void can_output_transmit(EventArbiterOutputData data) {
  LOG_DEBUG("Device = %d, State = %d, Data = %d\n",
             data.id,
             data.state,
             data.data);
}
