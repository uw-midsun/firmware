#include "driver_callback.h"
#include "input_event.h"
#include "event_queue.h"
#include "driver_io.h"

// Arbitrary thresholds for gas pedal
#define COAST_THRESHOLD 1000
#define DRIVE_THRESHOLD 3000

typedef struct AnalogStatus {
  InputEvent pedal;
} AnalogStatus;

static AnalogStatus s_analog_status;

static void prv_event(DriverIOData *driver_io_data, Event *e, GPIOState key_pressed) {
  switch (driver_io_data->id) {
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
      e->id = driver_io_data->event;
      return;
  }
}

void driver_callback_pedal(ADCChannel adc_channel, void *context) {
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

void driver_callback_input(GPIOAddress *address, void *context) {
  GPIOState key_pressed;
  gpio_get_value(address, &key_pressed);

  Event e = { .id = INPUT_EVENT_NONE };
  prv_event(context, &e, key_pressed);
  event_raise(e.id, e.data);
}
