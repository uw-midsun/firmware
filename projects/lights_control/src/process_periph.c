#include "status.h"
#include "gpio.h"
#include "event_queue.h"
#include "gpio_addresses.h"
#include "lights_events.h"

#define ADD_CASE(device) \
  case EVENT_##device: \
    return gpio_set_state(&ADDRESS_##device, (e.data) ? GPIO_STATE_HIGH : GPIO_STATE_LOW ); \
    break;

// Takes Event as input, uses the data field
// to determine whether to turn on or turn off
// the peripheral
StatusCode process_peripheral(Event e) {
  switch (e.id) {
    ADD_CASE(HORN)
    ADD_CASE(HEADLIGHTS)
    ADD_CASE(BRAKE)
    ADD_CASE(STROBE)
  }
  return STATUS_CODE_OK;
}

