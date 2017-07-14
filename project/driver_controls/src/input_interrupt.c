#include "input_interrupt.h"
#include "input_event.h"
#include "event_queue.h"
#include "driver_io.h"

// Number of available GPIO interrupt channels
#define NUM_INPUT_EVENTS 16

// Arbitrary thresholds for gas pedal
#define COAST_THRESHOLD 1000
#define DRIVE_THRESHOLD 3000

// Driver IO pin definitions
#define DRIVER_PIN_0  0
#define DRIVER_PIN_1  1
#define DRIVER_PIN_2  2
#define DRIVER_PIN_3  3
#define DRIVER_PIN_4  4
#define DRIVER_PIN_5  5
#define DRIVER_PIN_6  6
#define DRIVER_PIN_7  7
#define DRIVER_PIN_8  8
#define DRIVER_PIN_9  9
#define DRIVER_PIN_10 10
#define DRIVER_PIN_11 11
#define DRIVER_PIN_12 12
#define DRIVER_PIN_13 13
#define DRIVER_PIN_14 14
#define DRIVER_PIN_15 15

typedef void (*DriverInput)(GPIOAddress address, Event *e);

typedef struct AnalogStatus {
  InputEvent pedal;
} AnalogStatus;

static void prv_direction_selector(GPIOAddress address, Event *e);
static void prv_turn_signal(GPIOAddress address, Event *e);
static void prv_mechanical_brake(GPIOAddress address, Event *e);
static void prv_event(GPIOAddress address, Event *e);

static AnalogStatus s_analog_status;
static bool s_input_status[NUM_INPUT_EVENTS];

static DriverInput s_driver_inputs[NUM_DRIVER_IO_INPUTS];

static void prv_direction_selector(GPIOAddress address, Event *e) {
  uint8_t status = (s_input_status[DRIVER_PIN_3] << 1) | s_input_status[DRIVER_PIN_2];

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
  uint8_t status = (s_input_status[DRIVER_PIN_8] << 1) | s_input_status[DRIVER_PIN_7];

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

void input_interrupt_init() {
  s_driver_inputs[DRIVER_IO_POWER_SWITCH] = prv_event;
  s_driver_inputs[DRIVER_IO_GAS_PEDAL] = NULL;
  s_driver_inputs[DRIVER_IO_DIRECTION_SELECTOR] = prv_direction_selector;
  s_driver_inputs[DRIVER_IO_CRUISE_CONTROL] = prv_event;
  s_driver_inputs[DRIVER_IO_CRUISE_CONTROL_INC] = prv_event;
  s_driver_inputs[DRIVER_IO_CRUISE_CONTROL_DEC] = prv_event;
  s_driver_inputs[DRIVER_IO_TURN_SIGNAL] = prv_turn_signal;
  s_driver_inputs[DRIVER_IO_HAZARD_LIGHT] = prv_event;
  s_driver_inputs[DRIVER_IO_MECHANICAL_BRAKE] = prv_mechanical_brake;
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
}

void input_callback(GPIOAddress *address, void *context) {
  GPIOState key_pressed;
  Event e = { .id = INPUT_EVENT_NONE };

  gpio_get_value(address, &key_pressed);
  debounce(address, &key_pressed);

  s_input_status[address->pin] = key_pressed;

  s_driver_inputs[*(uint8_t*)context]((*address), &e);
  event_raise(e.id, e.data);
}
