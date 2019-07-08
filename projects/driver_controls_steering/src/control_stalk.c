#include "control_stalk.h"

#include <string.h>

#include "adc.h"
#include "config.h"
#include "event_queue.h"
#include "exported_enums.h"
#include "gpio_it.h"
#include "interrupt.h"
#include "soft_timer.h"

#include "can_transmit.h"

#define CONTROL_STALK_ADC_SAMPLE_PERIOD_MILLIS 50u

// Resistor divider value in ohms
#define CONTROL_STALK_RESISTOR_OHMS 1000u
// 4096 codes for +/-4.096V -> LSB = 2mV
#define CONTROL_STALK_THRESHOLD(ohms) \
  ((1u << 12) * (ohms) / ((CONTROL_STALK_RESISTOR_OHMS) + (ohms)))
// 2k181 +10% resistor = ~2k4, -10% = 1k963
#define CONTROL_STALK_2181_OHMS_THRESHOLD CONTROL_STALK_THRESHOLD(1963)
// 681 +10% resistor = ~750, -10% = 613
#define CONTROL_STALK_681_OHMS_THRESHOLD CONTROL_STALK_THRESHOLD(613)

static void prv_adc_callback(SoftTimerId timer_id, void *context) {
  ControlStalkStorage *storage = (ControlStalkStorage *)context;

  for (size_t i = 0; i < SIZEOF_ARRAY(storage->analog_config); ++i) {
    AdcChannel channel = NUM_ADC_CHANNELS;
    adc_get_channel(storage->analog_config[i].address, &channel);

    uint16_t reading = 0;
    adc_read_converted(channel, &reading);

    ControlStalkState state = CONTROL_STALK_STATE_FLOATING;
    if (reading <= CONTROL_STALK_681_OHMS_THRESHOLD) {
      state = CONTROL_STALK_STATE_681_OHMS;
    } else if (reading <= CONTROL_STALK_2181_OHMS_THRESHOLD) {
      state = CONTROL_STALK_STATE_2181_OHMS;
    }

    // If the previous state has changed, then we send an input
    if (storage->prev_analog_state[channel] != state) {
      // Raise event on the master via a CAN message
      CAN_TRANSMIT_STEERING_EVENT(storage->analog_config[i].can_event[state], 0);
    }
    storage->prev_analog_state[channel] = state;
  }

  // Start next soft timer
  soft_timer_start_millis(CONTROL_STALK_ADC_SAMPLE_PERIOD_MILLIS, prv_adc_callback, storage, NULL);
}

void prv_gpio_callback(const GpioAddress *address, void *context) {
  GpioState state = NUM_GPIO_STATES;
  gpio_get_state(address, &state);

  EventId *can_event = context;
  // Raise event on the master via a CAN message
  CAN_TRANSMIT_STEERING_EVENT(can_event[(size_t)state], 0);
}

StatusCode control_stalk_init(ControlStalkStorage *storage) {
  // Clear the storage
  memset(storage->prev_analog_state, NUM_CONTROL_STALK_STATES, sizeof(storage->prev_analog_state));

  // Initialize all GPIO pins for Digital Inputs
  GpioSettings digital_input_settings = {
    .direction = GPIO_DIR_IN,         //
    .state = GPIO_STATE_LOW,          //
    .resistor = GPIO_RES_NONE,        //
    .alt_function = GPIO_ALTFN_NONE,  //
  };
  for (size_t i = 0; i < SIZEOF_ARRAY(storage->digital_config); ++i) {
    status_ok_or_return(gpio_init_pin(&storage->digital_config[i].pin, &digital_input_settings));

    InterruptSettings interrupt_settings = {
      .type = INTERRUPT_TYPE_INTERRUPT,      //
      .priority = INTERRUPT_PRIORITY_NORMAL  //
    };
    // Initialize GPIO Interrupts to raise events to change
    status_ok_or_return(gpio_it_register_interrupt(
        &storage->digital_config[i].pin, &interrupt_settings, INTERRUPT_EDGE_RISING_FALLING,
        prv_gpio_callback, storage->digital_config[i].can_event));
  }

  // Initialize all GPIO pins for Analog Inputs
  GpioSettings adc_input_settings = {
    .direction = GPIO_DIR_IN,           //
    .state = GPIO_STATE_LOW,            //
    .resistor = GPIO_RES_NONE,          //
    .alt_function = GPIO_ALTFN_ANALOG,  //
  };
  for (size_t i = 0; i < SIZEOF_ARRAY(storage->analog_config); ++i) {
    status_ok_or_return(gpio_init_pin(&storage->analog_config[i].address, &adc_input_settings));
    AdcChannel channel = NUM_ADC_CHANNELS;

    // Enable ADC channel
    status_ok_or_return(adc_get_channel(storage->analog_config[i].address, &channel));
    status_ok_or_return(adc_set_channel(channel, true));
  }

  status_ok_or_return(soft_timer_start_millis(CONTROL_STALK_ADC_SAMPLE_PERIOD_MILLIS,
                                              prv_adc_callback, storage, NULL));

  return STATUS_CODE_OK;
}
