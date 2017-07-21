#include <stddef.h>

#include "analog_io.h"
#include "adc.h"
#include "gpio.h"
#include "event_queue.h"
#include "input_event.h"

// Arbitrary thresholds for gas pedal
#define ANALOG_IO_COAST_THRESHOLD 1000
#define ANALOG_IO_DRIVE_THRESHOLD 3000

// Arbitrary threshold for mechanical brake
#define ANALOG_IO_BRAKE_THRESHOLD 2047

// Max number of devices based on external ADC channels
#define ANALOG_IO_DEVICES 16

// Analog device identifiers
typedef enum {
  ANALOG_IO_DEVICE_GAS_PEDAL = 0,
  ANALOG_IO_DEVICE_MECHANICAL_BRAKE,
  NUM_ANALOG_IO_DEVICE
} AnalogIODevice;

// Lookup table for device ids based on ADC channel
static AnalogIODevice s_analog_devices[ANALOG_IO_DEVICES];

static void prv_input_callback(ADCChannel adc_channel, void *context) {
  Event e;

  adc_read_raw(adc_channel, &e.data);

  switch (s_analog_devices[adc_channel]) {
    case ANALOG_IO_DEVICE_GAS_PEDAL:
      if (e.data < ANALOG_IO_COAST_THRESHOLD) {
        e.id = INPUT_EVENT_PEDAL_BRAKE;
      } else if (e.data > ANALOG_IO_DRIVE_THRESHOLD) {
        e.id = INPUT_EVENT_PEDAL_PRESSED;
      } else {
        e.id = INPUT_EVENT_PEDAL_COAST;
      }
      break;
    case ANALOG_IO_DEVICE_MECHANICAL_BRAKE:
      e.id = (e.data > ANALOG_IO_BRAKE_THRESHOLD) ?
              INPUT_EVENT_MECHANICAL_BRAKE_PRESSED :
              INPUT_EVENT_MECHANICAL_BRAKE_RELEASED;
      break;
  }

  event_raise(e.id, e.data);
}

void analog_io_init() {
  typedef struct InputConfig {
    GPIOAddress address;
    AnalogIODevice device;
  } InputConfig;

  ADCChannel adc_channel;

  InputConfig analog_inputs[] = {
    { .address = { GPIO_PORT_C, 1 }, .device = ANALOG_IO_DEVICE_GAS_PEDAL },
    { .address = { GPIO_PORT_A, 0 }, .device = ANALOG_IO_DEVICE_MECHANICAL_BRAKE }
  };

  GPIOSettings settings = { GPIO_DIR_IN, GPIO_STATE_LOW, GPIO_RES_NONE, GPIO_ALTFN_NONE };

  for (uint8_t i = 0; i < SIZEOF_ARRAY(analog_inputs); i++) {
    adc_get_channel(analog_inputs[i].address, &adc_channel);
    adc_set_channel(adc_channel, true);
    adc_register_callback(adc_channel, prv_input_callback, NULL);

    s_analog_devices[adc_channel] = analog_inputs[i].device;
  }
}
