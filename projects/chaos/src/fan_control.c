#include "fan_control.h"

#include <stdbool.h>

#include "can_transmit.h"
#include "chaos_events.h"
#include "event_queue.h"
#include "exported_enums.h"

bool fan_controls_process_event(const Event *e) {
  if (e->id == CHAOS_EVENT_SEQUENCE_IDLE_DONE) {
    CAN_TRANSMIT_FAN_CONTROL(EE_FAN_CONTROL_STATE_DISABLE);
    return true;
  }
  return false;
}
