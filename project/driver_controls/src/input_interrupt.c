#include "adc.h"
#include "input_interrupt.h"
#include "event_queue.h"
#include "stm32f0xx.h"

#define NUM_INPUT_EVENTS 16

#define COAST_THRESHOLD 1000
#define DRIVE_THRESHOLD 3000

typedef void (*DriverInput)(GPIOAddress address, Event *e);

typedef struct AnalogStatus {
  InputEvent pedal;
} AnalogStatus;

static void prv_direction_selector(GPIOAddress address, Event *e);
static void prv_turn_signal(GPIOAddress address, Event *e);
static void prv_mechanical_brake(GPIOAddress address, Event *e);
static void prv_event(GPIOAddress address, Event *e);

static AnalogStatus s_analog_status;

// Possibly replace with uint16_t later on
static bool s_input_status[16];

static const DriverInput s_driver_inputs[] = {
  prv_event,
  NULL,
  prv_direction_selector,
  prv_direction_selector,
  prv_event,
  prv_event,
  prv_event,
  prv_turn_signal,
  prv_turn_signal,
  prv_event,
  prv_mechanical_brake
};

static InputEvent s_event_lookup[NUM_INPUT_EVENTS] = {
  INPUT_EVENT_POWER,
  INPUT_EVENT_GAS_BRAKE,
  INPUT_EVENT_GAS_COAST,
  INPUT_EVENT_GAS_PRESSED,
  INPUT_EVENT_CRUISE_CONTROL,
  INPUT_EVENT_CRUISE_CONTROL_INC,
  INPUT_EVENT_CRUISE_CONTROL_DEC,
  INPUT_EVENT_HAZARD_LIGHT
};

static prv_update_status(GPIOAddress address, GPIOState key_pressed) {
  s_input_status[address.pin] = key_pressed;
}

static void prv_direction_selector(GPIOAddress address, Event *e) {
  uint8_t status = (s_input_status[3] << 1) | s_input_status[2];

  switch (status) {
    case 0:
      e->id = INPUT_EVENT_DIRECTION_SELECTOR_NEUTRAL;
      return;
    case 1:
      e->id = INPUT_EVENT_DIRECTION_SELECTOR_DRIVE;
      return;
    case 2:
      e->id = INPUT_EVENT_DIRECTION_SELECTOR_REVERSE;
      return;
  }
}

static void prv_turn_signal(GPIOAddress address, Event *e) {
  uint8_t status = (s_input_status[8] << 1) | s_input_status[7];

  switch (status) {
    case 0:
      e->id = INPUT_EVENT_TURN_SIGNAL_NONE;
      return;
    case 1:
      e->id = INPUT_EVENT_TURN_SIGNAL_LEFT;
      return;
    case 2:
      e->id = INPUT_EVENT_TURN_SIGNAL_RIGHT;
      return;
  }
}

static void prv_mechanical_brake(GPIOAddress address, Event *e) {
  e->id = INPUT_EVENT_MECHANICAL_BRAKE;
}

static void prv_event(GPIOAddress address, Event *e) {
  GPIOState key_pressed;
  gpio_get_value(&address, &key_pressed);

  if (key_pressed == GPIO_STATE_HIGH) {
    switch (address.pin) {
      case 0:
        e->id = INPUT_EVENT_POWER;
        break;
      case 4:
        e->id = INPUT_EVENT_CRUISE_CONTROL;
        break;
      case 5:
        e->id = INPUT_EVENT_CRUISE_CONTROL_INC;
        break;
      case 6:
        e->id = INPUT_EVENT_CRUISE_CONTROL_DEC;
        break;
      case 9:
        e->id = INPUT_EVENT_HAZARD_LIGHT;
        break;
    }
  }
}

void pedal_callback(ADCChannel adc_channel, void *context) {
  Event e;

  adc_read_raw(adc_channel, &e.data);

  if (e.data < COAST_THRESHOLD) {
    e.id = INPUT_EVENT_GAS_BRAKE;
  } else if (e.data > DRIVE_THRESHOLD) {
    e.id = INPUT_EVENT_GAS_PRESSED;
  } else {
    e.id = INPUT_EVENT_GAS_COAST;
  }

  if (e.id != s_analog_status.pedal) {
    event_raise(e.id, e.data);
    s_analog_status.pedal = e.id;
  }
  return;
}

void input_callback(GPIOAddress *address, void *context) {
  GPIOState key_pressed;
  Event e = { .id = INPUT_EVENT_NONE };

  gpio_get_value(address, &key_pressed);
  debounce(address, &key_pressed);

  prv_update_status(*address, key_pressed);

  s_driver_inputs[address->pin]((*address), &e);
  event_raise(e.id, e.data);
  return;
}
