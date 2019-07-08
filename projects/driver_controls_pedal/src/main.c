#include <stdbool.h>

#include "config.h"

#include "event_queue.h"
#include "gpio.h"
#include "gpio_it.h"
#include "interrupt.h"
#include "soft_timer.h"
#include "status.h"
#include "wait.h"

#include "can.h"
#include "can_msg_defs.h"
#include "can_unpack.h"

#include "ads1015.h"
#include "exported_enums.h"
#include "pedal_calib.h"
#include "pedal_events.h"
#include "throttle.h"

static PedalCalibBlob s_calib_blob;
static CanStorage s_can_storage = { 0 };
static Ads1015Storage s_pedal_ads1015;

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

  StatusCode status = NUM_STATUS_CODES;
  Event e = { 0 };
  while (true) {
    if (status_ok(event_process(&e))) {
      can_process_event(&e);
    }
  }

  return 0;
}
