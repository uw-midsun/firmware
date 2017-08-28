#include "can_output.h"
#include "event_arbiter.h"
#include "input_event.h"
#include "log.h"

void can_output_transmit(uint8_t device_id, uint8_t device_state, uint16_t device_data) {
  printf("Device = %d, State = %d, Data = %d\n",
          device_id,
          device_state,
          device_data);
}
