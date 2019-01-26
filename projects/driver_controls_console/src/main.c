#include "cc_cfg.h"

#include "power_fsm.h"
#include "direction_fsm.h"
#include "headlights_fsm.h"
#include "hazards_fsm.h"

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
static Fsm s_fsm[NUM_CONSOLE_CONTROLS_FSMS];

static CanStorage s_can;

int main() {
  gpio_init();
  interrupt_init();
  gpio_it_init();
  soft_timer_init();
  event_queue_init();
  // unknown if needed at this time
  // crc32_init();
  // flash_init();

  const CanSettings can_settings {
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

  can_add_filter(SYSTEM_CAN_MESSAGE_POWER_STATE);

  center_console_init(&s_console);

  drive_output_init(drive_output_global(), INPUT_EVENT_DRIVE_WATCHDOG_FAULT,
                    INPUT_EVENT_DRIVE_UPDATE_REQUESTED);

  event_arbiter_init(&s_event_arbiter);
  DriverControlsFsmInitFn init_fns[] = {
    direction_fsm_init,
    power_fsm_init,
    headlight_fsm_init,
    turn_signal_fsm_init,
    hazards_fsm_init,
  };
  for (size_t i = 0; i < NUM_CONSOLE_CONTROLS_FSMS; i++) {
    init_fns[i](&s_fsms[i], &s_event_arbiter);
  }

  // Init Green Blinking LED?

  LOG_DEBUG("Console Controls initialized\n");

  Event e;
  while(true) {
    if (status_ok(event_process(&e))) {
#ifdef CC_CFG_DEBUG_PRINT_EVENTS
      LOG_DEBUG("e %d %d\n", e.id, e.data);
#endif

      can_process_event(&e);
      power_distribution_controller_retry(&e); // Needed for powering up?
      event_arbiter_process_event(&s_event_arbiter, &e);
    }
  }
}
