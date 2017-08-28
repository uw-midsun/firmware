#include "can_output.h"
#include "event_arbiter.h"
#include "input_event.h"
#include "log.h"

void can_output_transmit(uint8_t device_id, uint8_t device_state, uint16_t device_data) {
  union EventData {
    uint16_t raw;
    struct {
      uint16_t data:12;
      uint8_t state:4;
    } components;
  } data;

  data.components.data = device_data;
  data.components.state = device_state;

  printf("Device = %d, State = %d, Data = %d\n",
          device_id,
          data.components.state,
          data.components.data);
}
