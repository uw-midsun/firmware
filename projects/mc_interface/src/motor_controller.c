#include "motor_controller.h"
#include <math.h>
#include <stddef.h>
#include <string.h>
#include "can_transmit.h"
#include "critical_section.h"
#include "log.h"
#include "soft_timer.h"
#include "wavesculptor.h"

// Torque control mode:
// - velocity = +/-100 m/s
// - current = 0 to 100% (throttle position)
// Velocity control mode:
// - velocity = target m/s
// - current = 100%
// Regen braking:
// - velocity = 0
// - current = braking force

static void prv_bus_measurement_rx(const GenericCanMsg *msg, void *context) {
  // This should only be registered to the controller deemed to be the source of truth during cruise
  MotorControllerStorage *storage = context;
  WaveSculptorCanData can_data = { .raw = msg->data };

  // copy setpoint
  storage->cruise_current_percentage =
      fabsf(can_data.bus_measurement.bus_current / storage->settings.max_bus_current);
  storage->cruise_is_braking = (can_data.bus_measurement.bus_current < 0.0f);
}

static void prv_velocity_measurement_rx(const GenericCanMsg *msg, void *context) {
  MotorControllerStorage *storage = context;

  for (size_t i = 0; i < NUM_MOTOR_CONTROLLERS; i++) {
    WaveSculptorCanId can_id = { .raw = msg->id };
    if (can_id.device_id == storage->settings.ids[i].motor_controller) {
      WaveSculptorCanData can_data = { .raw = msg->data };
      storage->speed_cms[i] = (int16_t)(can_data.velocity_measurement.vehicle_velocity_ms * 100);
      storage->rx_bitset |= 1 << i;
      break;
    }
  }

  if (storage->rx_bitset == (1 << NUM_MOTOR_CONTROLLERS) - 1) {
    // Received speed from all motor controllers - clear bitset and broadcast
    storage->rx_bitset = 0;
    storage->settings.speed_cb(storage->speed_cms, NUM_MOTOR_CONTROLLERS,
                               storage->settings.context);
  }
}

static void prv_periodic_tx(SoftTimerID timer_id, void *context) {
  MotorControllerStorage *storage = context;

  GenericCanMsg msg = { .dlc = 8, .extended = false };

  for (size_t i = 0; i < NUM_MOTOR_CONTROLLERS; i++) {
    WaveSculptorCanId can_id = {
      .device_id = storage->settings.ids[i].interface,
      .msg_id = WAVESCULPTOR_CMD_ID_DRIVE,
    };
    msg.id = can_id.raw;

    WaveSculptorCanData can_data = { 0 };
    if (storage->timeout_counter > MOTOR_CONTROLLER_WATCHDOG_COUNTER) {
      // We haven't received an update from driver controls in a while - we'll do the safe thing
      // and send coast
      can_data.drive_cmd.motor_velocity_ms = 0.0f;
      can_data.drive_cmd.motor_current_percentage = 0.0f;
    } else if (storage->target_mode == MOTOR_CONTROLLER_MODE_TORQUE || i == 0) {
      can_data.drive_cmd.motor_velocity_ms = storage->target_velocity_ms;
      can_data.drive_cmd.motor_current_percentage = storage->target_current_percentage;
    } else {
      // Velocity control (non-primary): use torque control instead, but copy the current setpoint
      // from the primary motor controller
      can_data.drive_cmd.motor_velocity_ms =
          (storage->cruise_is_braking) ? 0.0f : WAVESCULPTOR_FORWARD_VELOCITY;
      can_data.drive_cmd.motor_current_percentage = storage->cruise_current_percentage;
    }
    msg.data = can_data.raw;

    generic_can_tx(storage->settings.can_uart, &msg);
  }
  storage->timeout_counter++;

  soft_timer_start_millis(MOTOR_CONTROLLER_DRIVE_TX_PERIOD_MS, prv_periodic_tx, storage, NULL);
}

StatusCode motor_controller_init(MotorControllerStorage *controller,
                                 const MotorControllerSettings *settings) {
  memset(controller, 0, sizeof(*controller));
  controller->settings = *settings;

  WaveSculptorCanId can_id = {
    .device_id = controller->settings.ids[0].motor_controller,
    .msg_id = WAVESCULPTOR_MEASUREMENT_ID_BUS,
  };

  // Only bother registering the bus measurement handler for the first motor controller since
  // that's all we care about
  generic_can_register_rx(controller->settings.can_uart, prv_bus_measurement_rx,
                          GENERIC_CAN_EMPTY_MASK, can_id.raw, false, controller);

  for (size_t i = 0; i < NUM_MOTOR_CONTROLLERS; i++) {
    can_id.device_id = controller->settings.ids[i].motor_controller;
    can_id.msg_id = WAVESCULPTOR_MEASUREMENT_ID_VELOCITY;
    generic_can_register_rx(controller->settings.can_uart, prv_velocity_measurement_rx,
                            GENERIC_CAN_EMPTY_MASK, can_id.raw, false, controller);
  }

  return soft_timer_start_millis(MOTOR_CONTROLLER_DRIVE_TX_PERIOD_MS, prv_periodic_tx, controller,
                                 NULL);
}

StatusCode motor_controller_set_speed_cb(MotorControllerStorage *controller,
                                         MotorControllerSpeedCb speed_cb, void *context) {
  bool disabled = critical_section_start();
  controller->settings.speed_cb = speed_cb;
  controller->settings.context = context;
  critical_section_end(disabled);

  return STATUS_CODE_OK;
}

StatusCode motor_controller_set_throttle(MotorControllerStorage *controller, int16_t throttle,
                                         EEDriveOutputDirection direction) {
  // Use impossible target velocity for torque control
  const float velocity_lookup[] = {
    [EE_DRIVE_OUTPUT_DIRECTION_NEUTRAL] = 0.0f,
    [EE_DRIVE_OUTPUT_DIRECTION_FORWARD] = WAVESCULPTOR_FORWARD_VELOCITY,
    [EE_DRIVE_OUTPUT_DIRECTION_REVERSE] = WAVESCULPTOR_REVERSE_VELOCITY,
  };

  float target_velocity = velocity_lookup[direction];

  if (direction == EE_DRIVE_OUTPUT_DIRECTION_NEUTRAL) {
    // Neutral - coast
    throttle = 0;
  } else if (throttle < 0) {
    // Braking - velocity = 0, brake %
    throttle *= -1;
    target_velocity = 0.0f;
  }

  // Reset counter
  bool disabled = critical_section_start();
  controller->timeout_counter = 0;
  controller->target_velocity_ms = target_velocity,
  controller->target_current_percentage = (float)throttle / EE_DRIVE_OUTPUT_DENOMINATOR;
  controller->target_mode = MOTOR_CONTROLLER_MODE_TORQUE;
  critical_section_end(disabled);

  return STATUS_CODE_OK;
}

StatusCode motor_controller_set_cruise(MotorControllerStorage *controller, int16_t speed_cms) {
  // Cruise control is always forward

  bool disabled = critical_section_start();
  controller->timeout_counter = 0;
  controller->target_velocity_ms = (float)speed_cms / 100;
  controller->target_current_percentage = 1.0f;

  // Start with no information on cruise state
  controller->cruise_current_percentage = 0.0f;
  controller->cruise_is_braking = false;
  controller->target_mode = MOTOR_CONTROLLER_MODE_VELOCITY;
  critical_section_end(disabled);

  return STATUS_CODE_OK;
}
