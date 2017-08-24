#include "input_event.h"
#include <stdio.h>

StatusCode input_event_raise_can(InputEvent device_id, uint8_t device_state, uint16_t device_data) {
  union EventData {
    uint16_t raw;
    struct {
      uint16_t data : 12;
      uint8_t state : 4;
    } components;
  } data;

  data.components.data = device_data;
  data.components.state = device_state;

  event_raise(device_id, data.raw);

  return STATUS_CODE_OK;
}
