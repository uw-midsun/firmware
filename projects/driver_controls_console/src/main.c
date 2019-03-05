#include "can.h"
#include "gpio.h"
#include "gpio_it.h"
#include "i2c.h"
#include "interrupt.h"
#include "log.h"

#include "bps_indicator.h"
#include "cc_cfg.h"
#include "cc_input_event.h"
#include "center_console.h"
#include "console_output.h"
#include "event_arbiter.h"
#include "heartbeat_rx.h"
#include "led_output.h"
#include "mech_brake_indicator.h"
#include "power_distribution_controller.h"

#include "direction_fsm.h"
#include "hazards_fsm.h"
#include "headlight_fsm.h"
#include "power_fsm.h"

typedef StatusCode (*ConsoleControlsFsmInitFn)(Fsm *fsm, EventArbiterStorage *storage);

typedef enum {
  CONSOLE_CONTROLS_FSM_POWER = 0,
  CONSOLE_CONTROLS_FSM_DIRECTION,
  CONSOLE_CONTROLS_FSM_HEADLIGHTS,
  CONSOLE_CONTROLS_FSM_HAZARDS,
  NUM_CONSOLE_CONTROLS_FSMS,
} ConsoleControlsFsm;

static CenterConsoleStorage s_console;
static EventArbiterStorage s_event_arbiter;
static Fsm s_fsms[NUM_CONSOLE_CONTROLS_FSMS];

static CanStorage s_can;
static HeartbeatRxHandlerStorage s_powertrain_heartbeat;
static GpioExpanderStorage s_led_expander;

int main(void) {
  gpio_init();
  interrupt_init();
  gpio_it_init();
  soft_timer_init();
  event_queue_init();
  // unknown if needed at this time
  // crc32_init();
  // flash_init();

  const CanSettings can_settings = {
    .device_id = CC_CFG_CAN_DEVICE_ID,
    .bitrate = CC_CFG_CAN_BITRATE,
    .rx_event = INPUT_EVENT_CAN_RX,
    .tx_event = INPUT_EVENT_CAN_TX,
    .fault_event = INPUT_EVENT_CAN_FAULT,
    .tx = CC_CFG_CAN_RX,
    .rx = CC_CFG_CAN_TX,
    .loopback = false,
  };
  can_init(&s_can, &can_settings);
  can_add_filter(SYSTEM_CAN_MESSAGE_BPS_HEARTBEAT);
  can_add_filter(SYSTEM_CAN_MESSAGE_POWER_STATE);
  can_add_filter(SYSTEM_CAN_MESSAGE_POWERTRAIN_HEARTBEAT);

  // GPIO Expander for LEDs
  const I2CSettings i2c_settings = {
    .speed = I2C_SPEED_FAST,    //
    .sda = CC_CFG_I2C_BUS_SDA,  //
    .scl = CC_CFG_I2C_BUS_SCL,  //
  };
  i2c_init(CC_CFG_I2C_BUS_PORT, &i2c_settings);
  gpio_expander_init(&s_led_expander, CC_CFG_I2C_BUS_PORT, CC_CFG_CONSOLE_IO_ADDR, NULL);
  led_output_init(&s_led_expander);

  center_console_init(&s_console);

  // BPS heartbeat
  bps_indicator_init();

  // Not sure that this does anything since NULL context is being passed
  // and none of the functions called on callback raise an event
  // Powertrain heartbeat
  heartbeat_rx_register_handler(&s_powertrain_heartbeat, SYSTEM_CAN_MESSAGE_POWERTRAIN_HEARTBEAT,
                                heartbeat_rx_auto_ack_handler, NULL);

  // Mech Brake
  mech_brake_indicator_init();

  console_output_init(console_output_global(), INPUT_EVENT_CONSOLE_WATCHDOG_FAULT,
                      INPUT_EVENT_CONSOLE_UPDATE_REQUESTED);

  event_arbiter_init(&s_event_arbiter);
  ConsoleControlsFsmInitFn init_fns[] = { direction_fsm_init, power_fsm_init, headlight_fsm_init,
                                          hazards_fsm_init };
  for (size_t i = 0; i < NUM_CONSOLE_CONTROLS_FSMS; i++) {
    init_fns[i](&s_fsms[i], &s_event_arbiter);
  }

  // Init Green Blinking LED?

  LOG_DEBUG("Console Controls initialized\n");

  Event e;
  while (true) {
    if (status_ok(event_process(&e))) {
#ifdef CC_CFG_DEBUG_PRINT_EVENTS
      LOG_DEBUG("e %d %d\n", e.id, e.data);
#endif

      can_process_event(&e);
      power_distribution_controller_retry(&e);
      event_arbiter_process_event(&s_event_arbiter, &e);
      led_output_process_event(&e);
    }
  }
}
