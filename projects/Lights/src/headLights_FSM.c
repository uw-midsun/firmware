#pragma once

#include "can_output.h"
#include "event_arbiter.h"
#include "input_event.h"
#include "log.h"

if (x){ //where x represents the canbus message instructions
        //not sure how to account for that
  canbus_message_direction = true;
} else {
  canbus_message_direction = false;
}

GPIOAddress headLights_gpio{
  port = port_address; //not sure how you would like me to assign pins/ports
  pin = headLights_pin;
}

GPIOSettings headLights_pin{
  direction GPIO_DIR_OUT;
	state = GPIO_STATE_LOW;
	resistor; // how should I assign a resistor?
	alt_function; // whats this?
}

StatusCode gpio_init_pin(&headLights_GPIO, &headLights_pin);

typedef enum{
  HEADLIGHTS_STATE_ON,
  HEADLIGHTS_STATE_OFF,
}headLightsState;

headLightsState current_headLights_state = HeadLights_STATE_OFF;

if (canbus_message_direction) {
  current_headLights_state = HeadLights_STATE_ON;
  somePin->state = GPIO_STATE_HIGH;
  while (canbus_message_direction) {
  }
  current_headLights_state = HeadLights_STATE_OFF;
  somePin->state = GPIO_STATE_LOW;
}

//free gpios here?
