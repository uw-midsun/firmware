#include <stddef.h>

#include "analog_io.h"
#include "adc.h"
#include "event_queue.h"
#include "input_event.h"

// Arbitrary thresholds for gas pedal
#define COAST_THRESHOLD 1000
#define DRIVE_THRESHOLD 3000

// Device identifiers
typedef enum {
  ANALOG_GAS_PEDAL = 0,
  NUM_ANALOG_IO_INPUTS
} AnalogIODevices;

typedef struct AnalogStatus {
  InputEvent pedal;
} AnalogStatus;

static AnalogStatus s_analog_status;

static AnalogIODevices s_analog_devices[] = {
  [ADC_CHANNEL_11] = ANALOG_GAS_PEDAL
};

static void prv_input_callback(ADCChannel adc_channel, void *context) {
  Event e;

  adc_read_raw(adc_channel, &e.data);

  printf("%d\n", e.data);

  switch (s_analog_devices[adc_channel]) {
    case ANALOG_GAS_PEDAL:
      if (e.data < COAST_THRESHOLD) { e.id = INPUT_EVENT_GAS_BRAKE; }
      else if (e.data > DRIVE_THRESHOLD) { e.id = INPUT_EVENT_GAS_PRESSED; }
      else { e.id = INPUT_EVENT_GAS_COAST; }
      break;
  }

  if (e.id != s_analog_status.pedal) {
    event_raise(e.id, e.data);
    s_analog_status.pedal = e.id;
  }
}

static ADCChannel prv_get_channel(GPIOAddress address) {
  ADCChannel adc_channel;

  switch(address.port) {
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
  ADCChannel adc_channel;

  GPIOAddress analog_inputs[] = {
    { GPIO_PORT_C, 1 }
  };

  GPIOSettings settings = { GPIO_DIR_IN, GPIO_STATE_LOW, GPIO_RES_NONE, GPIO_ALTFN_NONE };

  for (uint8_t i = 0; i < SIZEOF_ARRAY(analog_inputs); i++) {
    adc_channel = prv_get_channel(analog_inputs[i]);

    adc_set_channel(adc_channel, 1);
    adc_register_callback(adc_channel, prv_input_callback, NULL);
  }
}
