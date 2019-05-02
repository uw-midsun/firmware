#include <stdbool.h>

#include "gpio.h"
#include "gpio_it.h"
#include "interrupt.h"
#include "soft_timer.h"
#include "status.h"
#include "wait.h"

#include "event_queue.h"

#include "can.h"
#include "can_msg_defs.h"
#include "can_unpack.h"

#include "exported_enums.h"
#include "input_event.h"

#include "ads1015.h"
#include "pedal_calib.h"
#include "throttle.h"

#include "brake_signal.h"

#include "config.h"

#include "cruise.h"
#include "drive_output.h"
#include "event_arbiter.h"
#include "fsm.h"
#include "fsm/cruise_fsm.h"
#include "fsm/direction_fsm.h"
#include "fsm/hazards_fsm.h"
#include "fsm/headlight_fsm.h"
#include "fsm/horn_fsm.h"
#include "fsm/mechanical_brake_fsm.h"
#include "fsm/pedal_fsm.h"
#include "fsm/power_fsm.h"
#include "fsm/turn_signal_fsm.h"

#include "log.h"
typedef enum {
  DRIVER_CONTROLS_FSM_POWER = 0,
  DRIVER_CONTROLS_FSM_CRUISE,
  DRIVER_CONTROLS_FSM_PEDAL,
  DRIVER_CONTROLS_FSM_DIRECTION,
  DRIVER_CONTROLS_FSM_MECH_BRAKE,
  DRIVER_CONTROLS_FSM_HEADLIGHT,
  DRIVER_CONTROLS_FSM_TURN_SIGNALS,
  DRIVER_CONTROLS_FSM_HAZARDS,
  DRIVER_CONTROLS_FSM_HORN,
  NUM_DRIVER_CONTROLS_FSMS,
} DriverControlsFsm;

static PedalCalibBlob s_calib_blob;
static CanStorage s_can_storage;
static Ads1015Storage s_pedal_ads1015;
static EventArbiterStorage s_event_arbiter;
static Fsm s_fsms[NUM_DRIVER_CONTROLS_FSMS];

typedef StatusCode (*DriverControlsFsmInitFn)(Fsm *fsm, EventArbiterStorage *storage);

// Center Console events
static const EventId s_center_console_digital_event_mapping[NUM_EE_CENTER_CONSOLE_DIGITAL_INPUTS] =
    {
      [EE_CENTER_CONSOLE_DIGITAL_INPUT_LOW_BEAM] = PEDAL_EVENT_INPUT_CENTER_CONSOLE_LOWBEAMS,
      [EE_CENTER_CONSOLE_DIGITAL_INPUT_HAZARDS] = PEDAL_EVENT_INPUT_CENTER_CONSOLE_HAZARDS_PRESSED,
      [EE_CENTER_CONSOLE_DIGITAL_INPUT_DRL] = PEDAL_EVENT_INPUT_CENTER_CONSOLE_DRL,
      [EE_CENTER_CONSOLE_DIGITAL_INPUT_DRIVE] = PEDAL_EVENT_INPUT_CENTER_CONSOLE_DIRECTION_DRIVE,
      [EE_CENTER_CONSOLE_DIGITAL_INPUT_NEUTRAL] =
          PEDAL_EVENT_INPUT_CENTER_CONSOLE_DIRECTION_NEUTRAL,
      [EE_CENTER_CONSOLE_DIGITAL_INPUT_REVERSE] =
          PEDAL_EVENT_INPUT_CENTER_CONSOLE_DIRECTION_REVERSE,
      [EE_CENTER_CONSOLE_DIGITAL_INPUT_POWER] = PEDAL_EVENT_INPUT_CENTER_CONSOLE_POWER_PRESSED  //
    };

static const EventId s_steering_event_mapping[NUM_EE_STEERING_INPUTS] = {
  [EE_STEERING_INPUT_HORN_PRESSED] = PEDAL_EVENT_INPUT_CONTROL_STALK_DIGITAL_HORN_PRESSED,
  [EE_STEERING_INPUT_HORN_RELEASED] = PEDAL_EVENT_INPUT_CONTROL_STALK_DIGITAL_HORN_RELEASED,
  [EE_STEERING_INPUT_CC_ON_OFF_PRESSED] = PEDAL_EVENT_INPUT_CONTROL_STALK_DIGITAL_CC_ON,
  [EE_STEERING_INPUT_CC_ON_OFF_RELEASED] = PEDAL_EVENT_INPUT_CONTROL_STALK_DIGITAL_CC_OFF,
  [EE_STEERING_INPUT_CC_SET_PRESSED] = PEDAL_EVENT_INPUT_CONTROL_STALK_DIGITAL_CC_SET_PRESSED,
  [EE_STEERING_INPUT_CC_SET_RELEASED] = PEDAL_EVENT_INPUT_CONTROL_STALK_DIGITAL_CC_SET_RELEASED,

  // Analog events
  [EE_STEERING_INPUT_CC_SPEED_NEUTRAL] = PEDAL_EVENT_INPUT_CONTROL_STALK_ANALOG_CC_SPEED_NEUTRAL,
  [EE_STEERING_INPUT_CC_SPEED_MINUS] = PEDAL_EVENT_INPUT_CONTROL_STALK_ANALOG_CC_SPEED_MINUS,
  [EE_STEERING_INPUT_CC_SPEED_PLUS] = PEDAL_EVENT_INPUT_CONTROL_STALK_ANALOG_CC_SPEED_PLUS,

  [EE_STEERING_INPUT_CC_CANCEL_RESUME_NEUTRAL] = PEDAL_EVENT_INPUT_CONTROL_STALK_ANALOG_CC_NEUTRAL,
  [EE_STEERING_INPUT_CC_CANCEL_RESUME_CANCEL] = PEDAL_EVENT_INPUT_CONTROL_STALK_ANALOG_CC_CANCEL,
  [EE_STEERING_INPUT_CC_CANCEL_RESUME_RESUME] = PEDAL_EVENT_INPUT_CONTROL_STALK_ANALOG_CC_RESUME,

  [EE_STEERING_INPUT_TURN_SIGNAL_STALK_NONE] =
      PEDAL_EVENT_INPUT_CONTROL_STALK_ANALOG_TURN_SIGNAL_NONE,
  [EE_STEERING_INPUT_TURN_SIGNAL_STALK_RIGHT] =
      PEDAL_EVENT_INPUT_CONTROL_STALK_ANALOG_TURN_SIGNAL_RIGHT,
  [EE_STEERING_INPUT_TURN_SIGNAL_STALK_LEFT] =
      PEDAL_EVENT_INPUT_CONTROL_STALK_ANALOG_TURN_SIGNAL_LEFT,

  [EE_STEERING_INPUT_EVENT_CC_DISTANCE_NEUTRAL] =
      PEDAL_EVENT_INPUT_CONTROL_STALK_ANALOG_CC_DISTANCE_NEUTRAL,
  [EE_STEERING_INPUT_EVENT_CC_DISTANCE_MINUS] =
      PEDAL_EVENT_INPUT_CONTROL_STALK_ANALOG_CC_DISTANCE_MINUS,
  [EE_STEERING_INPUT_EVENT_CC_DISTANCE_PLUS] =
      PEDAL_EVENT_INPUT_CONTROL_STALK_ANALOG_CC_DISTANCE_PLUS,
};

static StatusCode prv_steering_rx_handler(const CanMessage *msg, void *context,
                                          CanAckStatus *ack_reply) {
  uint16_t event_id = 0u;
  uint16_t event_data = 0u;

  CAN_UNPACK_STEERING_EVENT(msg, &event_id, &event_data);

  if (!(event_id < SIZEOF_ARRAY(s_steering_event_mapping))) {
    LOG_DEBUG("Steering: CAN to Local event mapping out of range\n");
    return status_code(STATUS_CODE_OUT_OF_RANGE);
  }

  event_raise(s_steering_event_mapping[event_id], 0);
  return STATUS_CODE_OK;
}

static StatusCode prv_center_console_rx_handler(const CanMessage *msg, void *context,
                                                CanAckStatus *ack_reply) {
  uint16_t event_id = 0;
  uint16_t data = 0;
  CAN_UNPACK_CENTER_CONSOLE_EVENT(msg, &event_id, &data);

  if (!(event_id < SIZEOF_ARRAY(s_center_console_digital_event_mapping))) {
    LOG_DEBUG("Center Console: CAN to Local event mapping out of range\n");
    return status_code(STATUS_CODE_OUT_OF_RANGE);
  }

  event_raise(s_center_console_digital_event_mapping[event_id], 0);
  return STATUS_CODE_OK;
}

int main() {
  // Standard module inits
  gpio_init();
  interrupt_init();
  gpio_it_init();
  soft_timer_init();
  event_queue_init();

  // CAN initialization
  const CanSettings can_settings = {
    .device_id = SYSTEM_CAN_DEVICE_DRIVER_CONTROLS_PEDAL,
    .bitrate = CAN_HW_BITRATE_500KBPS,
    .rx_event = PEDAL_EVENT_CAN_RX,
    .tx_event = PEDAL_EVENT_CAN_TX,
    .fault_event = PEDAL_EVENT_CAN_FAULT,
    .tx = { GPIO_PORT_A, 12 },
    .rx = { GPIO_PORT_A, 11 },
  };
  status_ok_or_return(can_init(&s_can_storage, &can_settings));

  // Initialize all FSMs and the Arbiter
  status_ok_or_return(event_arbiter_init(&s_event_arbiter));
  DriverControlsFsmInitFn init_fns[] = {
    cruise_fsm_init,      direction_fsm_init, mechanical_brake_fsm_init,
    pedal_fsm_init,       power_fsm_init,     headlight_fsm_init,
    turn_signal_fsm_init, hazards_fsm_init,   horn_fsm_init,
  };
  for (size_t i = 0; i < NUM_DRIVER_CONTROLS_FSMS; i++) {
    status_ok_or_return(init_fns[i](&s_fsms[i], &s_event_arbiter));
  }

  // Setup ADC readings
  I2CSettings i2c_settings = {
    .speed = I2C_SPEED_FAST,  //
    .scl = PEDAL_CONFIG_PIN_I2C_SDL,
    .sda = PEDAL_CONFIG_PIN_I2C_SDA,  //
  };
  status_ok_or_return(i2c_init(I2C_PORT_2, &i2c_settings));
  GpioAddress ready_pin = PEDAL_CONFIG_PIN_ADS1015_READY;
  status_ok_or_return(ads1015_init(&s_pedal_ads1015, I2C_PORT_2, ADS1015_ADDRESS_GND, &ready_pin));

  // Pedal
  // We expect to grab the calibration data, which will contain the mappings
  // for which channels are MAIN and SECONDARY
  // This should be ADS1015_CHANNEL_0 and ADS1015_CHANNEL_1
  status_ok_or_return(calib_init(&s_calib_blob, sizeof(s_calib_blob), false));
  PedalCalibBlob *pedal_calib_blob = calib_blob();
  status_ok_or_return(
      throttle_init(throttle_global(), &pedal_calib_blob->throttle_calib, &s_pedal_ads1015));

  // Mechanical Brake
  const MechBrakeSettings mech_brake_settings = {
    .ads1015 = &s_pedal_ads1015,
    .brake_pressed_threshold_percentage = 35,
    .bounds_tolerance_percentage = 5,
    .channel = ADS1015_CHANNEL_2,
  };
  status_ok_or_return(mech_brake_init(mech_brake_global(), &mech_brake_settings,
                                      &pedal_calib_blob->mech_brake_calib));

  status_ok_or_return(brake_signal_init());

  // Drive Output messages
  status_ok_or_return(drive_output_init(drive_output_global(),
                                        PEDAL_EVENT_INPUT_PEDAL_WATCHDOG_FAULT,
                                        PEDAL_EVENT_INPUT_DRIVE_UPDATE_REQUESTED));

  // Cruise
  status_ok_or_return(cruise_init(cruise_global()));

  // Register CAN RX handlers to handle data from Driver Controls slaves
  // Steering Interface
  status_ok_or_return(
      can_register_rx_handler(SYSTEM_CAN_MESSAGE_STEERING_EVENT, prv_steering_rx_handler, NULL));
  // Center Console
  status_ok_or_return(can_register_rx_handler(SYSTEM_CAN_MESSAGE_CENTER_CONSOLE_EVENT,
                                              prv_center_console_rx_handler, NULL));
  // TODO: Allocate a CAN message or provide a way to disable regen braking
  // behaviour.

  StatusCode status = NUM_STATUS_CODES;
  Event e = { 0 };
  while (true) {
    while (status_ok(event_process(&e))) {
      can_process_event(&e);
      event_arbiter_process_event(&s_event_arbiter, &e);

      brake_signal_process_event(&e);
      cruise_handle_event(cruise_global(), &e);
    }
  }

  return 0;
}
