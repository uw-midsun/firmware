
typedef StatusCode (*PedalControlsFsmInitFn)(Fsm *fsm, EventArbiterStorage *storage);

typedef enum {
  DRIVER_CONTROLS_FSM_PEDA = 0,
  DRIVER_CONTROLS_FSM_MECH_BRAKE,
  NUM_DRIVER_CONTROLS_FSMS,
} PedalControlsFsm;

static CanStorage s_can;
static Fsm s_fsms[NUM_PEDAL_CONTROLS_FSMS];

int main(void) {
  gpio_init();
  interrupt_init();
  gpio_it_init();
  soft_timer_init();
  event_queue_init();

  const CanSettings can_settings = {
    .device_id = PC_CFG_CAN_DEVICE_ID,
    .bitrate = PC_CFG_CAN_BITRATE,
    .rx_event = INPUT_EVENT_CAN_RX,
    .tx_event = INPUT_EVENT_CAN_TX,
    .fault_event = INPUT_EVENT_CAN_FAULT,
    .tx = PC_CFG_CAN_TX,
    .rx = PC_CFG_CAN_RX,
    .loopback = false,
  };
  can_init(&s_can, &can_settings);

  GpioAddress pedal_ads1015_ready = DC_CFG_PEDAL_ADC_RDY_PIN;
  ads1015_init(&s_pedal_ads1015, DC_CFG_I2C_BUS_PORT, DC_CFG_PEDAL_ADC_ADDR, &pedal_ads1015_ready);
  DcCalibBlob *dc_calib_blob = calib_blob();
  throttle_init(throttle_global(), &dc_calib_blob->throttle_calib, &s_pedal_ads1015);

  const MechBrakeSettings mech_brake_settings = {
    .ads1015 = &s_pedal_ads1015,
    .brake_pressed_threshold_percentage = 55,
    .brake_unpressed_threshold_percentage = 45,
    .bounds_tolerance_percentage = 5,
    .channel = ADS1015_CHANNEL_2,
  };
  mech_brake_init(mech_brake_global(), &mech_brake_settings, &dc_calib_blob->mech_brake_calib);

  cruise_init(cruise_global());
  drive_output_init(drive_output_global(), INPUT_EVENT_DRIVE_WATCHDOG_FAULT,
                    INPUT_EVENT_DRIVE_UPDATE_REQUESTED);
}
