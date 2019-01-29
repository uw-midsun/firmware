
typedef StatusCode (*SteeringControlsFsmInitFn)(Fsm *fsm, EventArbiterStorage *storage);

typedef enum {
  STEERING_CONTROLS_FSM_CRUISE = 0,
  STEERING_CONTROLS_FSM_TURN_SIGNALS,
  NUM_STEERING_CONTROLS_FSMS,
} ConsoleControlsFsm;

static CanStorage s_can;
static Fsm s_fsms[NUM_DRIVER_CONTROLS_FSMS];

int main(void) {
  gpio_init();
  interrupt_init();
  gpio_it_init();
  soft_timer_init();
  event_queue_init();

  const CanSettings can_settings = {
    .device_id = SC_CFG_CAN_DEVICE_ID,
    .bitrate = SC_CFG_CAN_BITRATE,
    .rx_event = INPUT_EVENT_CAN_RX,
    .tx_event = INPUT_EVENT_CAN_TX,
    .fault_event = INPUT_EVENT_CAN_FAULT,
    .tx = SC_CFG_CAN_TX,
    .rx = SC_CFG_CAN_RX,
    .loopback = false,
  };
  can_init(&s_can, &can_settings);

#ifndef SC_CFG_DISABLE_CONTROL_STALK
  GpioAddress stalk_int_pin = SC_CFG_STALK_IO_INT_PIN;
  GpioAddress stalk_ready_pin = SC_CFG_STALK_ADC_RDY_PIN;
  gpio_expander_init(&s_stalk_expander, SC_CFG_I2C_BUS_PORT, SC_CFG_STALK_IO_ADDR, &stalk_int_pin);
  ads1015_init(&s_stalk_ads1015, SC_CFG_I2C_BUS_PORT, SC_CFG_STALK_ADC_ADDR, &stalk_ready_pin);
  control_stalk_init(&s_stalk, &s_stalk_ads1015, &s_stalk_expander);
#endif
}
