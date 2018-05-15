#include "lights_blinker.h"
#include "lights_events.h"

static LightsEventGpioPeripheral s_blinker_peripheral = NUM_LIGHTS_EVENT_GPIO_PERIPHERALS;

// Mocking lights_blinker's behaviour.
StatusCode lights_blinker_init(LightsBlinker *blinker, LightsBlinkerDuration duration_ms) {
  return STATUS_CODE_OK;
}

StatusCode lights_blinker_activate(LightsBlinker *blinker, LightsEventGpioPeripheral peripheral) {
  if (s_blinker_peripheral != peripheral) {
    if (s_blinker_peripheral != NUM_LIGHTS_EVENT_GPIO_PERIPHERALS) {
      // If the blinker is already active, cancel the old one.
      status_ok_or_return(event_raise(LIGHTS_EVENT_GPIO_OFF, s_blinker_peripheral));
    }
    s_blinker_peripheral = peripheral;
    return event_raise(LIGHTS_EVENT_GPIO_ON, s_blinker_peripheral);
  }
  return STATUS_CODE_OK;
}

StatusCode lights_blinker_deactivate(LightsBlinker *blinker) {
  event_raise(LIGHTS_EVENT_GPIO_OFF, s_blinker_peripheral);
  s_blinker_peripheral = NUM_LIGHTS_EVENT_GPIO_PERIPHERALS;
  return STATUS_CODE_OK;
}

StatusCode lights_blinker_sync_on(LightsBlinker *blinker) {
  return STATUS_CODE_OK;
}
