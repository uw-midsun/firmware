#include "adc.h"
#include "input_interrupt.h"
#include "event_queue.h"
#include "stm32f0xx.h"

#define NUM_INPUT_EVENTS 16

#define COAST_THRESHOLD 819
#define DRIVE_THRESHOLD 2662

typedef void (*DriverInput)(GPIOState *key_pressed, Event *e);

static void prv_power_switch(GPIOState *key_pressed, Event *e);
static void prv_direction_selector(GPIOState *key_pressed, Event *e);
static void prv_cruise_control_switch(GPIOState *key_pressed, Event *e);
static void prv_cruise_control_inc(GPIOState *key_pressed, Event *e);
static void prv_cruise_control_dec(GPIOState *key_pressed, Event *e);
static void prv_turn_signal(GPIOState *key_pressed, Event *e);
static void prv_hazard_light(GPIOState *key_pressed, Event *e);
static void prv_mechanical_brake(GPIOState *key_pressed, Event *e);

static const DriverInput s_driver_inputs[NUM_INPUT_EVENTS] = {
  prv_power_switch,
  NULL,
  prv_direction_selector,
  prv_direction_selector,
  prv_cruise_control_switch,
  prv_cruise_control_inc,
  prv_cruise_control_dec,
  prv_turn_signal,
  prv_turn_signal,
  prv_hazard_light,
  prv_mechanical_brake
};

static void prv_power_switch(GPIOState *key_pressed, Event *e) {
  if (*key_pressed) {
    e->id = INPUT_EVENT_POWER;
  }
}
static void prv_direction_selector(GPIOState *key_pressed, Event *e) {
  switch ((GPIOB->IDR & (GPIO_IDR_2 | GPIO_IDR_3)) >> 2) {
    case 0:
      e->id = INPUT_EVENT_DIRECTION_SELECTOR_NEUTRAL;
      return;
    case 1:
      e->id = INPUT_EVENT_DIRECTION_SELECTOR_DRIVE;
      return;
    case 2:
      e->id = INPUT_EVENT_DIRECTION_SELECTOR_REVERSE;
      return;
  }
}

static void prv_cruise_control_switch(GPIOState *key_pressed, Event *e) {
  if (*key_pressed) {
    e->id = INPUT_EVENT_CRUISE_CONTROL;
  }
}

static void prv_cruise_control_inc(GPIOState *key_pressed, Event *e) {
  if (*key_pressed) {
    e->id = INPUT_EVENT_CRUISE_CONTROL_INC;
  }
}

static void prv_cruise_control_dec(GPIOState *key_pressed, Event *e) {
  if (*key_pressed) {
    e->id = INPUT_EVENT_CRUISE_CONTROL_DEC;
  }
}

static void prv_turn_signal(GPIOState *key_pressed, Event *e) {
  switch ((GPIOC->IDR & (GPIO_IDR_7 | GPIO_IDR_8)) >> 7) {
    case 0:
      e->id = INPUT_EVENT_TURN_SIGNAL_NONE;
      return;
    case 1:
      e->id = INPUT_EVENT_TURN_SIGNAL_LEFT;
      return;
    case 2:
      e->id = INPUT_EVENT_TURN_SIGNAL_RIGHT;
      return;
  }
}

static void prv_hazard_light(GPIOState *key_pressed, Event *e) {
  if (*key_pressed) {
    e->id = INPUT_EVENT_HAZARD_LIGHT;
  }
}

static void prv_mechanical_brake(GPIOState *key_pressed, Event *e) {
  e->id = INPUT_EVENT_MECHANICAL_BRAKE;
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

  event_raise(e.id, e.data);
  return;
}

void input_callback(GPIOAddress *address, void *context) {
  GPIOState key_pressed;
  Event e = { .id = INPUT_EVENT_NONE };

  gpio_get_value(address, &key_pressed);
  debounce(address, &key_pressed);

  s_driver_inputs[address->pin](&key_pressed, &e);
  event_raise(e.id, 0);
  return;
}
