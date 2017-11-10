#include <stddef.h>
#include <stdio.h>

#include "steering_wheel_io.h"
#include "driver_io.h"
#include "gpio_it.h"
#include "input_event.h"
#include "gpio_expander.h"
#include "interrupt_def.h"

// Steeing wheel input identifiers
typedef enum {
  STEERING_WHEEL_IO_TURN_SIGNAL = 0,
  STEERING_WHEEL_IO_CRUISE_CONTROL,
  STEERING_WHEEL_IO_CRUISE_CONTROL_INC,
  STEERING_WHEEL_IO_CRUISE_CONTROL_DEC,
  STEERING_WHEEL_IO_PUSH_TO_TALK,
  STEERING_WHEEL_IO_BMS_FAULT,
  STEERING_WHEEL_IO_HORN
} SteeringWheelIODevice;

// Store the id of the device as well as the id of the event the device raises
typedef struct SteeringWheelIOData {
  SteeringWheelIODevice id;
  InputEvent event;
  InterruptEdge edge;
} SteeringWheelIOData;

static SteeringWheelIOData s_steering_input_data[NUM_GPIO_EXPANDER_PINS];

static void prv_steering_wheel_callback(GPIOExpanderPin pin, GPIOState state, void *context) {
  SteeringWheelIOData *data = (SteeringWheelIOData *)context;

  gpio_expander_get_state(pin, &state);

  // Since the GPIO Expander causes an interrupt on both rising and falling edge, interrupts on
  // the incorrect edge must be ignored
  bool rising_edge_fail = (data->edge == INTERRUPT_EDGE_RISING) && (state != GPIO_STATE_HIGH);
  bool falling_edge_fail = (data->edge == INTERRUPT_EDGE_FALLING) && (state != GPIO_STATE_LOW);

  if (rising_edge_fail || falling_edge_fail) {
    return;
  }

  // Generate the event based on the input pin
  uint8_t event_id = data->event;

  if (data->id == STEERING_WHEEL_IO_TURN_SIGNAL && state == GPIO_STATE_LOW) {
    event_id = INPUT_EVENT_TURN_SIGNAL_NONE;
  }

  event_raise(event_id, 0);
  printf("Pin %d\n", pin);
  return;
}

// Configure gpio_expander pins
void steering_wheel_io_init(void) {
  // Define array to store configuration settings for each pin
  s_steering_input_data[DRIVER_IO_RIGHT_TURN_SIGNAL_PIN] = (SteeringWheelIOData){
        .id = STEERING_WHEEL_IO_TURN_SIGNAL,
        .event = INPUT_EVENT_TURN_SIGNAL_RIGHT,
        .edge = INTERRUPT_EDGE_RISING_FALLING
      };
  s_steering_input_data[DRIVER_IO_LEFT_TURN_SIGNAL_PIN] = (SteeringWheelIOData){
        .id = STEERING_WHEEL_IO_TURN_SIGNAL,
        .event = INPUT_EVENT_TURN_SIGNAL_LEFT,
        .edge = INTERRUPT_EDGE_RISING_FALLING
      };
  s_steering_input_data[DRIVER_IO_CRUISE_CONTROL_PIN] = (SteeringWheelIOData){
        .id = STEERING_WHEEL_IO_CRUISE_CONTROL,
        .event = INPUT_EVENT_CRUISE_CONTROL,
        .edge = INTERRUPT_EDGE_RISING
      };
  s_steering_input_data[DRIVER_IO_CRUISE_CONTROL_INC_PIN] = (SteeringWheelIOData){
        .id = STEERING_WHEEL_IO_CRUISE_CONTROL_INC,
        .event = INPUT_EVENT_CRUISE_CONTROL_INC,
        .edge = INTERRUPT_EDGE_RISING
      };
  s_steering_input_data[DRIVER_IO_CRUISE_CONTROL_DEC_PIN] = (SteeringWheelIOData){
        .id = STEERING_WHEEL_IO_CRUISE_CONTROL_DEC,
        .event = INPUT_EVENT_CRUISE_CONTROL_DEC,
        .edge = INTERRUPT_EDGE_RISING
      };
  s_steering_input_data[DRIVER_IO_PUSH_TO_TALK_PIN] = (SteeringWheelIOData){
        .id = STEERING_WHEEL_IO_PUSH_TO_TALK,
        .event = INPUT_EVENT_PUSH_TO_TALK,
        .edge = INTERRUPT_EDGE_RISING_FALLING
      };

  s_steering_input_data[DRIVER_IO_HORN_PIN] = (SteeringWheelIOData){
        .id = STEERING_WHEEL_IO_HORN,
        .event = INPUT_EVENT_HORN,
        .edge = INTERRUPT_EDGE_RISING_FALLING
      };

  GPIOSettings gpio_settings = {.direction = GPIO_DIR_IN, .state = GPIO_STATE_LOW };
  InterruptSettings it_settings = { INTERRUPT_TYPE_INTERRUPT, INTERRUPT_PRIORITY_LOW };

  // Initialize Steering Wheel inputs
  for (uint8_t i = 0; i < NUM_GPIO_EXPANDER_PINS; i++) {
    gpio_expander_init_pin(i, &gpio_settings);
    gpio_expander_register_callback(i, prv_steering_wheel_callback, &s_steering_input_data[i]);
  }
}
