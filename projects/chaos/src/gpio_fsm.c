#include "gpio_fsm.h"

#include <stdbool.h>

#include "chaos_config.h"
#include "chaos_events.h"
#include "event_queue.h"
#include "fsm.h"
#include "gpio.h"
#include "gpio_seq.h"
#include "misc.h"

#define GPIO_FSM_SLEW_RATE_US 10

static FSM s_gpio_fsm;

FSM_DECLARE_STATE(gpio_state_idle);
FSM_DECLARE_STATE(gpio_state_charge_preconfig);
FSM_DECLARE_STATE(gpio_state_charge);
FSM_DECLARE_STATE(gpio_state_drive_preconfig);
FSM_DECLARE_STATE(gpio_state_drive);
FSM_DECLARE_STATE(gpio_state_emergency);

FSM_STATE_TRANSITION(gpio_state_emergency) {
  FSM_ADD_TRANSITION(CHAOS_EVENT_GPIO_IDLE, gpio_state_idle);
  FSM_ADD_TRANSITION(CHAOS_EVENT_GPIO_CHARGE_PRECONFIG, gpio_state_charge_preconfig);
  FSM_ADD_TRANSITION(CHAOS_EVENT_GPIO_DRIVE_PRECONFIG, gpio_state_drive_preconfig);
  FSM_ADD_TRANSITION(CHAOS_EVENT_GPIO_EMERGENCY, gpio_state_emergency);
}

FSM_STATE_TRANSITION(gpio_state_idle) {
  FSM_ADD_TRANSITION(CHAOS_EVENT_GPIO_CHARGE_PRECONFIG, gpio_state_charge_preconfig);
  FSM_ADD_TRANSITION(CHAOS_EVENT_GPIO_DRIVE_PRECONFIG, gpio_state_drive_preconfig);
  FSM_ADD_TRANSITION(CHAOS_EVENT_GPIO_EMERGENCY, gpio_state_emergency);
}

FSM_STATE_TRANSITION(gpio_state_charge_preconfig) {
  FSM_ADD_TRANSITION(CHAOS_EVENT_GPIO_IDLE, gpio_state_idle);
  FSM_ADD_TRANSITION(CHAOS_EVENT_GPIO_CHARGE, gpio_state_charge);
  FSM_ADD_TRANSITION(CHAOS_EVENT_GPIO_DRIVE_PRECONFIG, gpio_state_drive_preconfig);
  FSM_ADD_TRANSITION(CHAOS_EVENT_GPIO_EMERGENCY, gpio_state_emergency);
}

FSM_STATE_TRANSITION(gpio_state_charge) {
  FSM_ADD_TRANSITION(CHAOS_EVENT_GPIO_IDLE, gpio_state_idle);
  FSM_ADD_TRANSITION(CHAOS_EVENT_GPIO_DRIVE_PRECONFIG, gpio_state_drive_preconfig);
  FSM_ADD_TRANSITION(CHAOS_EVENT_GPIO_EMERGENCY, gpio_state_emergency);
}

FSM_STATE_TRANSITION(gpio_state_drive_preconfig) {
  FSM_ADD_TRANSITION(CHAOS_EVENT_GPIO_IDLE, gpio_state_idle);
  FSM_ADD_TRANSITION(CHAOS_EVENT_GPIO_CHARGE_PRECONFIG, gpio_state_charge_preconfig);
  FSM_ADD_TRANSITION(CHAOS_EVENT_GPIO_DRIVE, gpio_state_drive);
  FSM_ADD_TRANSITION(CHAOS_EVENT_GPIO_EMERGENCY, gpio_state_emergency);
}

FSM_STATE_TRANSITION(gpio_state_drive) {
  FSM_ADD_TRANSITION(CHAOS_EVENT_GPIO_IDLE, gpio_state_idle);
  FSM_ADD_TRANSITION(CHAOS_EVENT_GPIO_CHARGE_PRECONFIG, gpio_state_charge_preconfig);
  FSM_ADD_TRANSITION(CHAOS_EVENT_GPIO_EMERGENCY, gpio_state_emergency);
}

static void prv_gpio_state_idle(FSM *fsm, const Event *e, void *context) {
  const ChaosConfig *cfg = context;
  const GPIOAddress sequence[] = {
    cfg->motor_interface_power, cfg->rear_camera_power,    cfg->front_lights_power,
    cfg->rear_lights_power,     cfg->driver_display_power, cfg->array_sense_power,
    cfg->battery_box_power,
  };

  gpio_seq_set_state(sequence, SIZEOF_ARRAY(sequence), GPIO_STATE_LOW, GPIO_FSM_SLEW_RATE_US);
}

static void prv_gpio_state_charge_preconfig(FSM *fsm, const Event *e, void *context) {
  const ChaosConfig *cfg = context;
  const GPIOAddress sequence[] = {
    cfg->front_lights_power,
    cfg->motor_interface_power,
    cfg->rear_camera_power,
  };

  gpio_seq_set_state(sequence, SIZEOF_ARRAY(sequence), GPIO_STATE_LOW, GPIO_FSM_SLEW_RATE_US);
}

static void prv_gpio_state_charge(FSM *fsm, const Event *e, void *context) {
  const ChaosConfig *cfg = context;
  const GPIOAddress sequence[] = {
    cfg->driver_display_power,
    cfg->array_sense_power,
    cfg->battery_box_power,
    cfg->rear_lights_power,
  };

  gpio_seq_set_state(sequence, SIZEOF_ARRAY(sequence), GPIO_STATE_HIGH, GPIO_FSM_SLEW_RATE_US);
}

static void prv_gpio_state_drive_preconfig(FSM *fsm, const Event *e, void *context) {
  const ChaosConfig *cfg = context;
  const GPIOAddress sequence[] = {
    cfg->battery_box_power,  cfg->driver_display_power, cfg->array_sense_power,
    cfg->front_lights_power, cfg->rear_lights_power,    cfg->motor_interface_power,
    cfg->rear_lights_power,
  };

  gpio_seq_set_state(sequence, SIZEOF_ARRAY(sequence), GPIO_STATE_LOW, GPIO_FSM_SLEW_RATE_US);
}

static void prv_gpio_state_drive(FSM *fsm, const Event *e, void *context) {
  const ChaosConfig *cfg = context;
  const GPIOAddress sequence[] = {
    cfg->battery_box_power,  cfg->driver_display_power, cfg->array_sense_power,
    cfg->front_lights_power, cfg->rear_lights_power,    cfg->motor_interface_power,
    cfg->rear_lights_power,
  };

  gpio_seq_set_state(sequence, SIZEOF_ARRAY(sequence), GPIO_STATE_HIGH, GPIO_FSM_SLEW_RATE_US);
}

void gpio_fsm_init(const ChaosConfig *cfg) {
  fsm_state_init(gpio_state_emergency,
                 prv_gpio_state_idle);  // Emergency should behave identically to Idle.
  fsm_state_init(gpio_state_idle, prv_gpio_state_idle);
  fsm_state_init(gpio_state_charge, prv_gpio_state_charge);
  fsm_state_init(gpio_state_drive, prv_gpio_state_drive);
  fsm_init(&s_gpio_fsm, "GpioFsm", &gpio_state_idle, (void *)cfg);

  GPIOSettings settings = {
    .direction = GPIO_DIR_OUT,
    .state = GPIO_STATE_HIGH,
    .resistor = GPIO_RES_NONE,
    .alt_function = GPIO_ALTFN_NONE,
  };
  const GPIOAddress init_high_sequence[] = {
    cfg->telemetry_power,
    cfg->themis_power,
  };

  gpio_seq_init_pins(init_high_sequence, SIZEOF_ARRAY(init_high_sequence), &settings,
                     GPIO_FSM_SLEW_RATE_US);

  const GPIOAddress init_low_sequence[] = {
    cfg->motor_interface_power, cfg->rear_camera_power,    cfg->front_lights_power,
    cfg->rear_lights_power,     cfg->driver_display_power, cfg->array_sense_power,
    cfg->battery_box_power,
  };
  settings.state = GPIO_STATE_LOW;

  gpio_seq_init_pins(init_low_sequence, SIZEOF_ARRAY(init_low_sequence), &settings,
                     GPIO_FSM_SLEW_RATE_US);
}

bool gpio_fsm_process_event(const Event *e) {
  return fsm_process_event(&s_gpio_fsm, e);
}
