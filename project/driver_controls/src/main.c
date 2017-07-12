#include <stdio.h>
#include <stdint.h>

#include "adc.h"
#include "input_interrupt.h"
#include "driver_state.h"
#include "soft_timer.h"
#include "driver_devices.h"

#include "power_state.h"
#include "pedal_state.h"
#include "direction_state.h"
#include "turn_signal_state.h"
#include "hazard_light_state.h"
#include "mechanical_brake.h"

#define INPUT_DEVICES 16
#define OUTPUT_DEVICES 1

// The struct of FSMs to be used in the system
typedef struct FSMGroup {
  FSM power;
  FSM pedal;
  FSM direction;
  FSM turn_signal;
  FSM hazard_light;
  FSM mechanical_brake;
} FSMGroup;

void device_init() {
  driver_controls_init();

  Device inputs[INPUT_DEVICES] = {
    { { GPIO_PORT_C, 0 }, GPIO_DIR_IN, INTERRUPT_EDGE_RISING,
      GPIO_ALTFN_NONE, input_callback },

    { { GPIO_PORT_C, 1 }, GPIO_DIR_IN, INTERRUPT_EDGE_RISING_FALLING,
      GPIO_ALTFN_ANALOG, input_callback },

    { { GPIO_PORT_B, 2 }, GPIO_DIR_IN, INTERRUPT_EDGE_RISING_FALLING,
      GPIO_ALTFN_NONE, input_callback },

    { { GPIO_PORT_B, 3 }, GPIO_DIR_IN, INTERRUPT_EDGE_RISING_FALLING,
      GPIO_ALTFN_NONE, input_callback },

    { { GPIO_PORT_C, 4 }, GPIO_DIR_IN, INTERRUPT_EDGE_RISING,
      GPIO_ALTFN_NONE, input_callback },

    { { GPIO_PORT_C, 5 }, GPIO_DIR_IN, INTERRUPT_EDGE_RISING,
      GPIO_ALTFN_NONE, input_callback },

    { { GPIO_PORT_C, 6 }, GPIO_DIR_IN, INTERRUPT_EDGE_RISING,
      GPIO_ALTFN_NONE, input_callback },

    { { GPIO_PORT_C, 7 }, GPIO_DIR_IN, INTERRUPT_EDGE_RISING_FALLING,
      GPIO_ALTFN_NONE, input_callback },

    { { GPIO_PORT_C, 8 }, GPIO_DIR_IN, INTERRUPT_EDGE_RISING_FALLING,
      GPIO_ALTFN_NONE, input_callback },

    { { GPIO_PORT_C, 9 }, GPIO_DIR_IN, INTERRUPT_EDGE_RISING,
      GPIO_ALTFN_NONE, input_callback },

    { { GPIO_PORT_C, 10 }, GPIO_DIR_IN, INTERRUPT_EDGE_RISING_FALLING,
      GPIO_ALTFN_NONE, input_callback }
  };

  Device outputs[OUTPUT_DEVICES] = {
    { { GPIO_PORT_C, 11 }, GPIO_DIR_OUT, 0, GPIO_ALTFN_NONE }
  };

  for (uint8_t i = 0; i < INPUT_DEVICES; i++) {
    driver_controls_add_device(&inputs[i]);
  }

  for (uint8_t i = 0; i < OUTPUT_DEVICES; i++) {
    driver_controls_add_device(&outputs[i]);
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

  // Initialize the GPIO inputs and other devices
  device_init();
  event_queue_init();
  soft_timer_init();

  adc_init(ADC_MODE_CONTINUOUS);
  adc_set_channel(ADC_CHANNEL_11, true);
  adc_register_callback(ADC_CHANNEL_11, pedal_callback, NULL);

  for (;;) {
    if (!event_process(&e)) {
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
