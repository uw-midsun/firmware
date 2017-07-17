#include "input_interrupt.h"
#include "input_event.h"
#include "event_queue.h"
#include "driver_io.h"

// Number of available GPIO interrupt channels
#define NUM_INPUT_EVENTS 16

// Arbitrary thresholds for gas pedal
#define COAST_THRESHOLD 1000
#define DRIVE_THRESHOLD 3000

typedef void (*DriverInput)(DriverIO *driver_io, Event *e);

typedef struct AnalogStatus {
  InputEvent pedal;
} AnalogStatus;

static AnalogStatus s_analog_status;
static bool s_input_status[NUM_INPUT_EVENTS];

static void prv_event(DriverIO *driver_io, Event *e, GPIOState key_pressed) {
  switch (driver_io->id) {
    case DRIVER_IO_DIRECTION_SELECTOR:
      if (key_pressed == GPIO_STATE_LOW) {
        e->id = INPUT_EVENT_DIRECTION_SELECTOR_NEUTRAL;
        return;
      }
    case DRIVER_IO_TURN_SIGNAL:
      if (key_pressed == GPIO_STATE_LOW) {
        e->id = INPUT_EVENT_TURN_SIGNAL_NONE;
        return;
      }
    case DRIVER_IO_POWER_SWITCH:
    case DRIVER_IO_CRUISE_CONTROL:
    case DRIVER_IO_CRUISE_CONTROL_INC:
    case DRIVER_IO_CRUISE_CONTROL_DEC:
    case DRIVER_IO_HAZARD_LIGHT:
    case DRIVER_IO_MECHANICAL_BRAKE:
      e->id = driver_io->event;
      return;
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
}

void input_callback(GPIOAddress *address, void *context) {
  GPIOState key_pressed;
  Event e = { .id = INPUT_EVENT_NONE };

  gpio_get_value(address, &key_pressed);

  s_input_status[address->pin] = key_pressed;

  prv_event(context, &e, key_pressed);

  event_raise(e.id, e.data);
}
