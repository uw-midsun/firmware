#pragma once
// Handles motor relay CAN requests and sequencing
// Requires CAN, soft timers to be initialized.
//
// Sequences relay closing with a slight delay to offset the high make current of the HV relay coil.
// We also assume the relays are active-high.
#include "gpio.h"
#include "relay_rx.h"
#include "soft_timer.h"
#include "status.h"

typedef struct MotorRelaySettings {
  GPIOAddress left_relay;
  GPIOAddress right_relay;
  // Delay between left and right relays closing
  uint32_t delay_ms;
} MotorRelaySettings;

typedef struct MotorRelayStorage {
  MotorRelaySettings settings;
  SoftTimerID delay_timer;
  RelayRxStorage relay_rx;
} MotorRelayStorage;

// |storage| should persist
StatusCode motor_relay_init(MotorRelayStorage *storage, const MotorRelaySettings *settings);
