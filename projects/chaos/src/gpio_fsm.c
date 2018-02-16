#include "gpio_fsm.h"

#include <stdbool.h>

#include "chaos_config.h"
#include "chaos_events.h"
#include "event_queue.h"
#include "fsm.h"
#include "gpio.h"
#include "gpio_seq.h"
#include "misc.h"

#define GPIO_FSM_SLEW_RATE 10

static FSM s_gpio_fsm;

FSM_DECLARE_STATE(idle);
FSM_DECLARE_STATE(charge);
FSM_DECLARE_STATE(drive);

FSM_STATE_TRANSITION(idle) {
  FSM_ADD_TRANSITION(CHAOS_EVENT_GPIO_CHARGE, charge);
  FSM_ADD_TRANSITION(CHAOS_EVENT_GPIO_DRIVE, drive);
}

FSM_STATE_TRANSITION(charge) {
  FSM_ADD_TRANSITION(CHAOS_EVENT_GPIO_IDLE, idle);
}

FSM_STATE_TRANSITION(drive) {
  FSM_ADD_TRANSITION(CHAOS_EVENT_GPIO_IDLE, idle);
}

static void prv_gpio_state_idle(FSM *fsm, const Event *e, void *context) {
  const ChaosConfig *cfg = context;
  const GPIOAddress sequence[] = {
    cfg->motor_interface_power, cfg->rear_camera_power,    cfg->front_lights_power,
    cfg->rear_lights_power,     cfg->driver_display_power, cfg->array_sense_power,
    cfg->battery_box_power,
  };

  gpio_seq_set_state(sequence, SIZEOF_ARRAY(sequence), GPIO_STATE_LOW, GPIO_FSM_SLEW_RATE);
}

static void prv_gpio_state_charge(FSM *fsm, const Event *e, void *context) {
  const ChaosConfig *cfg = context;
  const GPIOAddress sequence[] = {
    cfg->driver_display_power,
    cfg->array_sense_power,
    cfg->battery_box_power,
    cfg->rear_lights_power,
  };

  gpio_seq_set_state(sequence, SIZEOF_ARRAY(sequence), GPIO_STATE_HIGH, GPIO_FSM_SLEW_RATE);
}

static void prv_gpio_state_drive(FSM *fsm, const Event *e, void *context) {
  const ChaosConfig *cfg = context;
  const GPIOAddress sequence[] = {
    cfg->battery_box_power,  cfg->driver_display_power, cfg->array_sense_power,
    cfg->front_lights_power, cfg->rear_lights_power,    cfg->motor_interface_power,
    cfg->rear_lights_power,
  };

  gpio_seq_set_state(sequence, SIZEOF_ARRAY(sequence), GPIO_STATE_HIGH, GPIO_FSM_SLEW_RATE);
}

void gpio_fsm_init(const ChaosConfig *cfg) {
  fsm_state_init(idle, prv_gpio_state_idle);
  fsm_state_init(charge, prv_gpio_state_charge);
  fsm_state_init(drive, prv_gpio_state_drive);
  fsm_init(&s_gpio_fsm, "GpioFsm", &idle, cfg);

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
                     GPIO_FSM_SLEW_RATE);

  const GPIOAddress init_low_sequence[] = {
    cfg->motor_interface_power, cfg->rear_camera_power,    cfg->front_lights_power,
    cfg->rear_lights_power,     cfg->driver_display_power, cfg->array_sense_power,
    cfg->battery_box_power,
  };
  settings.state = GPIO_STATE_LOW;

  gpio_seq_init_pins(init_low_sequence, SIZEOF_ARRAY(init_low_sequence), &settings,
                     GPIO_FSM_SLEW_RATE);
}

bool gpio_fsm_process_event(const Event *e) {
  return fsm_process_event(&s_gpio_fsm, e);
}
