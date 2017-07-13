#include <stdio.h>
#include <stdint.h>

#include "adc.h"
#include "input_interrupt.h"
#include "driver_state.h"
#include "soft_timer.h"
#include "driver_device.h"

#include "power_state.h"
#include "pedal_state.h"
#include "direction_state.h"
#include "turn_signal_state.h"
#include "hazard_light_state.h"
#include "mechanical_brake_state.h"

#define INPUT_DEVICES 16
#define OUTPUT_DEVICES 1

// Struct of FSMs to be used in the program
typedef struct FSMGroup {
  FSM power;
  FSM pedal;
  FSM direction;
  FSM turn_signal;
  FSM hazard_light;
  FSM mechanical_brake;
} FSMGroup;

void device_init() {
  driver_device_init();

  // Configure driver devices with their individual settings
  DriverDevice inputs[INPUT_DEVICES] = {
    { .address = { GPIO_PORT_C, 0 }, .direction = GPIO_DIR_IN,
      .edge = INTERRUPT_EDGE_RISING,
      .alt_function = GPIO_ALTFN_NONE, .callback = input_callback },

    { .address = { GPIO_PORT_C, 1 }, .direction = GPIO_DIR_IN,
      .edge = INTERRUPT_EDGE_RISING_FALLING, .alt_function = GPIO_ALTFN_ANALOG,
      .callback = input_callback },

    { .address = { GPIO_PORT_B, 2 }, .direction = GPIO_DIR_IN,
      .edge = INTERRUPT_EDGE_RISING_FALLING,
      .alt_function = GPIO_ALTFN_NONE, .callback = input_callback },

    { .address = { GPIO_PORT_B, 3 }, .direction = GPIO_DIR_IN,
      .edge = INTERRUPT_EDGE_RISING_FALLING,
      .alt_function = GPIO_ALTFN_NONE, .callback = input_callback },

    { .address = { GPIO_PORT_C, 4 }, .direction = GPIO_DIR_IN,
      .edge = INTERRUPT_EDGE_RISING,
      .alt_function = GPIO_ALTFN_NONE, .callback = input_callback },

    { .address = { GPIO_PORT_C, 5 }, .direction = GPIO_DIR_IN,
      .edge = INTERRUPT_EDGE_RISING,
      .alt_function = GPIO_ALTFN_NONE, .callback = input_callback },

    { .address = { GPIO_PORT_C, 6 }, .direction = GPIO_DIR_IN,
      .edge = INTERRUPT_EDGE_RISING,
      .alt_function = GPIO_ALTFN_NONE, .callback = input_callback },

    { .address = { GPIO_PORT_C, 7 }, .direction = GPIO_DIR_IN,
      .edge = INTERRUPT_EDGE_RISING_FALLING,
      .alt_function = GPIO_ALTFN_NONE, .callback = input_callback },

    { .address = { GPIO_PORT_C, 8 }, .direction = GPIO_DIR_IN,
      .edge = INTERRUPT_EDGE_RISING_FALLING,
      .alt_function = GPIO_ALTFN_NONE, .callback = input_callback },

    { .address = { GPIO_PORT_C, 9 }, .direction = GPIO_DIR_IN,
      .edge = INTERRUPT_EDGE_RISING,
      .alt_function = GPIO_ALTFN_NONE, .callback = input_callback },

    { .address = { GPIO_PORT_C, 10 }, .direction = GPIO_DIR_IN,
      .edge = INTERRUPT_EDGE_RISING_FALLING,
      .alt_function = GPIO_ALTFN_NONE, .callback = input_callback }
  };

  DriverDevice outputs[OUTPUT_DEVICES] = {
    { .address = { GPIO_PORT_C, 11 },
      .direction = GPIO_DIR_OUT,
      .alt_function = GPIO_ALTFN_NONE }
  };

  for (uint8_t i = 0; i < INPUT_DEVICES; i++) {
    driver_device_init_pin(&inputs[i]);
  }

  for (uint8_t i = 0; i < OUTPUT_DEVICES; i++) {
    driver_device_init_pin(&outputs[i]);
  }
}

int main() {
  FSMGroup fsm_group;
  Event e;
  uint16_t reading;

  driver_state_add_fsm(&fsm_group.power, power_state_init);
  driver_state_add_fsm(&fsm_group.pedal, pedal_state_init);
  driver_state_add_fsm(&fsm_group.direction, direction_state_init);
  driver_state_add_fsm(&fsm_group.turn_signal, turn_signal_state_init);
  driver_state_add_fsm(&fsm_group.hazard_light, hazard_light_state_init);
  driver_state_add_fsm(&fsm_group.mechanical_brake, mechanical_brake_state_init);

  // Initialize the various driver control devices
  device_init();

  event_queue_init();
  soft_timer_init();

  adc_init(ADC_MODE_CONTINUOUS);
  adc_set_channel(ADC_CHANNEL_11, true);
  adc_register_callback(ADC_CHANNEL_11, pedal_callback, NULL);

  for (;;) {
    if (status_ok(event_process(&e))) {
      if (driver_state_process_event(&e)) {
        printf("Event = %d   :   %s   :   %s   :   %s   :   %s   :   %s   :   %s\n",
            e.id,
            fsm_group.power.current_state->name,
            fsm_group.pedal.current_state->name,
            fsm_group.direction.current_state->name,
            fsm_group.hazard_light.current_state->name,
            fsm_group.mechanical_brake.current_state->name,
            fsm_group.turn_signal.current_state->name);
      }
    }
  }
}
