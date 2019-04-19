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

#include "log.h"
static PedalCalibBlob s_calib_blob;
static CanStorage s_can_storage;
static Ads1015Storage s_pedal_ads1015;

static const EventId s_center_console_digital_event_mapping[NUM_EE_CENTER_CONSOLE_DIGITAL_INPUTS] =
    { [EE_CENTER_CONSOLE_DIGITAL_INPUT_LOW_BEAM] = PEDAL_EVENT_INPUT_CENTER_CONSOLE_LOWBEAMS,
      [EE_CENTER_CONSOLE_DIGITAL_INPUT_HAZARDS] = PEDAL_EVENT_INPUT_CENTER_CONSOLE_HAZARDS_PRESSED,
      [EE_CENTER_CONSOLE_DIGITAL_INPUT_DRL] = PEDAL_EVENT_INPUT_CENTER_CONSOLE_DRL,
      [EE_CENTER_CONSOLE_DIGITAL_INPUT_DRIVE] = PEDAL_EVENT_INPUT_CENTER_CONSOLE_DIRECTION_DRIVE,
      [EE_CENTER_CONSOLE_DIGITAL_INPUT_NEUTRAL] =
          PEDAL_EVENT_INPUT_CENTER_CONSOLE_DIRECTION_NEUTRAL,
      [EE_CENTER_CONSOLE_DIGITAL_INPUT_REVERSE] =
          PEDAL_EVENT_INPUT_CENTER_CONSOLE_DIRECTION_REVERSE,
      [EE_CENTER_CONSOLE_DIGITAL_INPUT_POWER] = PEDAL_EVENT_INPUT_CENTER_CONSOLE_POWER_PRESSED };

static const EventId
    s_steering_digital_event_mapping[NUM_EE_STEERING_DIGITAL_INPUTS][NUM_GPIO_STATES] = {
      [EE_STEERING_DIGITAL_INPUT_HORN] =
          {
              PEDAL_EVENT_INPUT_CONTROL_STALK_DIGITAL_HORN_PRESSED,  //
              PEDAL_EVENT_INPUT_CONTROL_STALK_DIGITAL_HORN_RELEASED  //
          },
      [EE_STEERING_DIGITAL_INPUT_CC_ON_OFF] =
          {
              PEDAL_EVENT_INPUT_CONTROL_STALK_DIGITAL_CC_ON,  //
              PEDAL_EVENT_INPUT_CONTROL_STALK_DIGITAL_CC_OFF  //
          },
      [EE_STEERING_DIGITAL_INPUT_CC_SET] =
          {
              PEDAL_EVENT_INPUT_CONTROL_STALK_DIGITAL_CC_SET_PRESSED,  //
              PEDAL_EVENT_INPUT_CONTROL_STALK_DIGITAL_CC_SET_RELEASED  //
          }                                                            //
    };

static const EventId s_steering_analog_event_mapping[NUM_EE_STEERING_ANALOG_INPUTS][3] = {
  [EE_STEERING_ANALOG_INPUT_CC_SPEED] =
      {
          PEDAL_EVENT_INPUT_CONTROL_STALK_ANALOG_CC_SPEED_NEUTRAL,
          PEDAL_EVENT_INPUT_CONTROL_STALK_ANALOG_CC_SPEED_MINUS,
          PEDAL_EVENT_INPUT_CONTROL_STALK_ANALOG_CC_SPEED_PLUS,
      },
  [EE_STEERING_ANALOG_INPUT_CC_CANCEL_RESUME] =
      {
          PEDAL_EVENT_INPUT_CONTROL_STALK_ANALOG_CC_NEUTRAL,
          PEDAL_EVENT_INPUT_CONTROL_STALK_ANALOG_CC_CANCEL,
          PEDAL_EVENT_INPUT_CONTROL_STALK_ANALOG_CC_RESUME,
      },
  [EE_STEERING_ANALOG_INPUT_TURN_SIGNAL_STALK] =
      {
          PEDAL_EVENT_INPUT_CONTROL_STALK_ANALOG_TURN_SIGNAL_NONE,
          PEDAL_EVENT_INPUT_CONTROL_STALK_ANALOG_TURN_SIGNAL_RIGHT,
          PEDAL_EVENT_INPUT_CONTROL_STALK_ANALOG_TURN_SIGNAL_LEFT,
      },
};

static StatusCode prv_steering_rx_handler(const CanMessage *msg, void *context,
                                          CanAckStatus *ack_reply) {
  uint16_t event_id = 0u;
  uint16_t event_data = 0u;

  CAN_UNPACK_STEERING_EVENT(msg, &event_id, &event_data);

  EventId can_to_local_event_mapping[] = {};

  if (!(button < SIZEOF_ARRAY(can_to_local_event_mapping))) {
    return status_code(STATUS_CODE_OUT_OF_RANGE);
  }

  event_raise(can_to_local_event_mapping[button], 0);

  return STATUS_CODE_OK;
}

static StatusCode prv_center_console_rx_handler(const CanMessage *msg, void *context,
                                                CanAckStatus *ack_reply) {
  uint16_t button = 0;
  CAN_UNPACK_CENTER_CONSOLE_EVENT(msg, &button);

  EventId can_to_local_event_mapping[] = {
    [EE_CENTER_CONSOLE_DIGITAL_INPUT_LOW_BEAM] = PEDAL_EVENT_INPUT_CENTER_CONSOLE_LOWBEAMS,
    [EE_CENTER_CONSOLE_DIGITAL_INPUT_DRL] = PEDAL_EVENT_INPUT_CENTER_CONSOLE_DRL,
    [EE_CENTER_CONSOLE_DIGITAL_INPUT_DRIVE] = PEDAL_EVENT_INPUT_CENTER_CONSOLE_DIRECTION_DRIVE,
    [EE_CENTER_CONSOLE_DIGITAL_INPUT_NEUTRAL] = PEDAL_EVENT_INPUT_CENTER_CONSOLE_DIRECTION_NEUTRAL,
    [EE_CENTER_CONSOLE_DIGITAL_INPUT_REVERSE] = PEDAL_EVENT_INPUT_CENTER_CONSOLE_DIRECTION_REVERSE,
    [EE_CENTER_CONSOLE_DIGITAL_INPUT_POWER] = PEDAL_EVENT_INPUT_CENTER_CONSOLE_POWER_PRESSED,
    [EE_CENTER_CONSOLE_DIGITAL_INPUT_HAZARDS] = PEDAL_EVENT_INPUT_CENTER_CONSOLE_HAZARDS_PRESSED
  };

  if (!(button < SIZEOF_ARRAY(can_to_local_event_mapping))) {
    return status_code(STATUS_CODE_OUT_OF_RANGE);
  }
  event_raise(can_to_local_event_mapping[button], 0);
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
    .device_id = SYSTEM_CAN_DEVICE_DRIVER_CONTROLS,  // TODO: Change
    .bitrate = CAN_HW_BITRATE_500KBPS,
    .rx_event = PEDAL_EVENT_CAN_RX,
    .tx_event = PEDAL_EVENT_CAN_TX,
    .fault_event = PEDAL_EVENT_CAN_FAULT,
    .tx = { GPIO_PORT_A, 12 },
    .rx = { GPIO_PORT_A, 11 },
  };
  status_ok_or_return(can_init(&s_can_storage, &can_settings));

  // Setup ADC readings
  I2CSettings i2c_settings = {
    .speed = I2C_SPEED_FAST,                    //
    .scl = { .port = GPIO_PORT_B, .pin = 10 },  //
    .sda = { .port = GPIO_PORT_B, .pin = 11 },  //
  };
  i2c_init(I2C_PORT_1, &i2c_settings);
  GpioAddress ready_pin = {
    .port = GPIO_PORT_B,  //
    .pin = 2,             //
  };
  ads1015_init(&s_pedal_ads1015, I2C_PORT_1, ADS1015_ADDRESS_GND, &ready_pin);

  // Pedal
  // We expect to grab the calibration data, which will contain the mappings
  // for which channels are MAIN and SECONDARY
  // This should be ADS1015_CHANNEL_0 and ADS1015_CHANNEL_1
  PedalCalibBlob *pedal_calib_blob = calib_blob();
  throttle_init(throttle_global(), &pedal_calib_blob->throttle_calib, &s_pedal_ads1015);

  // Mechanical Brake
  const MechBrakeSettings mech_brake_settings = {
    .ads1015 = &s_pedal_ads1015,
    .brake_pressed_threshold_percentage = 35,
    .bounds_tolerance_percentage = 5,
    .channel = ADS1015_CHANNEL_2,
  };
  mech_brake_init(mech_brake_global(), &mech_brake_settings, &pedal_calib_blob->mech_brake_calib);

  // Register CAN RX handlers to handle data from Driver Controls slaves
  // Steering Interface
  can_register_rx_handler(SYSTEM_CAN_MESSAGE_STEERING_DATA, prv_steering_rx_handler, NULL);
  // Center Console
  can_register_rx_handler(SYSTEM_CAN_MESSAGE_CENTER_CONSOLE_DATA, prv_center_console_rx_handler,
                          NULL);
  // TODO: Allocate a CAN message or provide a way to disable regen braking
  // behaviour.

  StatusCode status = NUM_STATUS_CODES;
  Event e = { 0 };
  while (true) {
    wait();

    while (status_ok(event_process(&e))) {
      can_process_event(&e);
      // event_arbiter_process_event(&s_event_arbiter, &e);
      // brake_signal_process_event(&e);
    }
  }

  return 0;
}
