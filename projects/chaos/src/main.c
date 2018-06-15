#include <stdbool.h>
#include <stdlib.h>

#include "adc.h"
#include "bps_heartbeat.h"
#include "can.h"
#include "can_ack.h"
#include "chaos_config.h"
#include "chaos_events.h"
#include "charger.h"
#include "debug_led.h"
#include "delay.h"
#include "delay_service.h"
#include "emergency_fault.h"
#include "event_queue.h"
#include "gpio.h"
#include "gpio_fsm.h"
#include "gpio_it.h"
#include "interrupt.h"
#include "log.h"
#include "power_path.h"
#include "powertrain_heartbeat.h"
#include "relay.h"
#include "relay_retry_service.h"
#include "sequencer_fsm.h"
#include "soft_timer.h"
#include "state_handler.h"
#include "status.h"
#include "wait.h"

#define CHAOS_DEBUG_LED_PERIOD_MS 500
#define CHAOS_NUM_RX_HANDLERS 10

static CANStorage s_can_storage;
static CANRxHandler s_rx_handlers[CHAOS_NUM_RX_HANDLERS];
static EmergencyFaultStorage s_emergency_storage;
static RelayRetryServiceStorage s_retry_storage;

static void prv_toggle(SoftTimerID id, void *context) {
  (void)id;
  (void)context;
  debug_led_toggle_state(DEBUG_LED_RED);
  soft_timer_start_millis(CHAOS_DEBUG_LED_PERIOD_MS, prv_toggle, NULL, NULL);
}

int main(void) {
  // Common
  event_queue_init();
  interrupt_init();
  soft_timer_init();
  gpio_init();
  gpio_it_init();
  adc_init(ADC_MODE_CONTINUOUS);
  debug_led_init(DEBUG_LED_RED);
  soft_timer_start_millis(CHAOS_DEBUG_LED_PERIOD_MS, prv_toggle, NULL, NULL);

  // CAN
  CANSettings can_settings = {
    .device_id = SYSTEM_CAN_DEVICE_CHAOS,
    .bitrate = CAN_HW_BITRATE_500KBPS,
    .rx_event = CHAOS_EVENT_CAN_RX,
    .tx_event = CHAOS_EVENT_CAN_TX,
    .fault_event = CHAOS_EVENT_CAN_FAULT,
    .tx = { GPIO_PORT_A, 12 },
    .rx = { GPIO_PORT_A, 11 },
    .loopback = false,
  };
  can_init(&s_can_storage, &can_settings, s_rx_handlers, SIZEOF_ARRAY(s_rx_handlers));

  // Heartbeats
  bps_heartbeat_init();  // Use the auto start feature to start the watchdog.
  powertrain_heartbeat_init();

  // Power Path
  ChaosConfig *cfg = chaos_config_load();
  power_path_init(&cfg->power_path);
  // AUX Battery Monitoring.
  power_path_source_monitor_enable(&cfg->power_path.aux_bat, CHAOS_CONFIG_POWER_PATH_PERIOD_MS);
  power_path_send_data_daemon(&cfg->power_path, CHAOS_CONFIG_POWER_PATH_PERIOD_MS);

  // Relays
  RelaySettings relay_settings = {
    .battery_main_power_pin = cfg->battery_box_power,
    .battery_slave_power_pin = cfg->battery_box_power,
    .motor_power_pin = cfg->motor_interface_power,
    .solar_front_power_pin = cfg->array_sense_power,
    .solar_rear_power_pin = cfg->array_sense_power,
    .loopback = false,
  };
  relay_init(&relay_settings);
  relay_retry_service_init(&s_retry_storage);

  // Sequencer
  sequencer_fsm_init();

  // Chaos is considered to be in the Idle state at this point and will only begin to transition
  // once it receives input from driver controls. To do so we enable the state handler and other
  // CAN services below now that Chaos is in what is considered to be a valid state.

  // CAN services
  charger_init();
  emergency_fault_init(&s_emergency_storage);
  state_handler_init();

  // GPIO
  // Postpone to as late as possible so that BPS heartbeats are ready to be ACK'd.
  gpio_fsm_init(cfg);

  // Main loop
  Event e = { 0 };
  StatusCode status = NUM_STATUS_CODES;
  while (true) {
    // Tight event loop
    do {
      status = event_process(&e);
      // TODO(ELEC-105): Validate nothing gets stuck here.
      if (status == STATUS_CODE_EMPTY) {
        wait();
      }
    } while (status != STATUS_CODE_OK);

    // Event Processing:

    // TODO(ELEC-105): At least one of the following should respond with either a boolean true or
    // a STATUS_CODE_OK for each emitted message. Consider adding a requirement that this is the
    // case with a failure resulting in faulting into Emergency.
    fsm_process_event(CAN_FSM, &e);
    delay_service_process_event(&e);
    emergency_fault_process_event(&s_emergency_storage, &e);
    gpio_fsm_process_event(&e);
    powertrain_heartbeat_process_event(&e);
    power_path_process_event(&cfg->power_path, &e);
    charger_process_event(&e);
    relay_process_event(&e);
    relay_retry_service_update(&e);
    sequencer_fsm_publish_next_event(&e);
  }

  // Not reached.
  return EXIT_SUCCESS;
}
