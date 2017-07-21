#include <stddef.h>

#include "analog_io.h"
#include "adc.h"
#include "gpio.h"
#include "event_queue.h"
#include "input_event.h"

// Arbitrary thresholds for gas pedal
#define ANALOG_IO_COAST_THRESHOLD 1000
#define ANALOG_IO_DRIVE_THRESHOLD 3000

// Max number of devices based on external ADC channels
#define ANALOG_IO_DEVICES 16

// Analog device identifiers
typedef enum {
  ANALOG_IO_DEVICE_GAS_PEDAL = 0,
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
        e.id = INPUT_EVENT_GAS_BRAKE;
      } else if (e.data > ANALOG_IO_DRIVE_THRESHOLD) {
        e.id = INPUT_EVENT_GAS_PRESSED;
      } else {
        e.id = INPUT_EVENT_GAS_COAST;
      }
      break;
  }

  event_raise(e.id, e.data);
}

// Obtain the ADC channel from the GPIO Address
static ADCChannel prv_get_channel(GPIOAddress address) {
  ADCChannel adc_channel;

  switch (address.port) {
    case GPIO_PORT_A:
      adc_channel += address.pin;
      break;
    case GPIO_PORT_B:
      adc_channel += (address.pin + 8);
      break;
    case GPIO_PORT_C:
      adc_channel += (address.pin + 10);
      break;
  }

  return adc_channel;
}

void analog_io_init() {
  typedef struct InputConfig {
    GPIOAddress address;
    AnalogIODevice device;
  } InputConfig;

  ADCChannel adc_channel;

  InputConfig analog_inputs[] = {
    { .address = { GPIO_PORT_C, 1 }, .device = ANALOG_IO_DEVICE_GAS_PEDAL }
  };

  GPIOSettings settings = { GPIO_DIR_IN, GPIO_STATE_LOW, GPIO_RES_NONE, GPIO_ALTFN_NONE };

  for (uint8_t i = 0; i < SIZEOF_ARRAY(analog_inputs); i++) {
    adc_get_channel(analog_inputs[i].address, &adc_channel);
    adc_set_channel(adc_channel, 1);
    adc_register_callback(adc_channel, prv_input_callback, NULL);
    
    s_analog_devices[adc_channel] = analog_inputs[i].device;
  }
}
