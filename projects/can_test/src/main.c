// CAN driver bus flooding test
// Attempts to flood the CAN bus - would like to ensure that the main thread
// doesn't starve
// and figure out how quickly we can realistically send messages
#include "can.h"
#include "gpio.h"
#include "interrupt.h"
#include "log.h"
#include "receiver.h"
#include "sender.h"
#include "soft_timer.h"

#define CAN_TEST_NUM_RX_HANDLERS 5
#define CAN_TEST_BUS_ACTIVE true

typedef enum {
  CAN_TEST_EVENT_RX = 0,
  CAN_TEST_EVENT_TX,
  CAN_TEST_EVENT_FAULT,
} CANTestEvent;

static GPIOAddress s_led = {GPIO_PORT_B, 3};
static CANStorage s_can_storage;
static CANRxHandler s_rx_handlers[CAN_TEST_NUM_RX_HANDLERS];

static void prv_blink_led(SoftTimerID timer_id, void *context) {
  gpio_toggle_state(&s_led);
  soft_timer_start_seconds(1, prv_blink_led, NULL, NULL);
}

static bool prv_is_sender(void) {
  GPIOAddress pin_out = {GPIO_PORT_A, 0};
  GPIOAddress pin_in = {GPIO_PORT_A, 1};

  GPIOSettings gpio_settings = {.direction = GPIO_DIR_OUT,
                                .state = GPIO_STATE_LOW};
  gpio_init_pin(&pin_out, &gpio_settings);

  gpio_settings.direction = GPIO_DIR_IN;
  gpio_settings.resistor = GPIO_RES_PULLUP;
  gpio_init_pin(&pin_in, &gpio_settings);

  GPIOState io_state = GPIO_STATE_LOW;
  gpio_get_state(&pin_in, &io_state);

  return io_state == GPIO_STATE_LOW;
}

int main(void) {
  gpio_init();
  interrupt_init();
  soft_timer_init();
  event_queue_init();

  GPIOSettings gpio_settings = {.direction = GPIO_DIR_OUT,
                                .state = GPIO_STATE_LOW};
  gpio_init_pin(&s_led, &gpio_settings);
  prv_blink_led(0, NULL);

  bool is_sender = prv_is_sender();
  LOG_DEBUG("Is sender: %d\n", is_sender);

  CANSettings can_settings = {
      .device_id = 0x4 + is_sender,
      .bitrate = CAN_HW_BITRATE_500KBPS,
      .rx_event = CAN_TEST_EVENT_RX,
      .tx_event = CAN_TEST_EVENT_TX,
      .fault_event = CAN_TEST_EVENT_FAULT,
      .tx = {GPIO_PORT_A, 12},
      .rx = {GPIO_PORT_A, 11},
  };
  can_init(&can_settings, &s_can_storage, s_rx_handlers,
           CAN_TEST_NUM_RX_HANDLERS);

  if (is_sender || CAN_TEST_BUS_ACTIVE) {
    uint16_t msg_id = 15 + !is_sender;
    sender_init(msg_id);
  }

  if (!is_sender) {
    receiver_init();
  }

  while (true) {
    Event e = {0};
    while (status_ok(event_process(&e))) {
      bool success = fsm_process_event(CAN_FSM, &e);
    }
  }

  return 0;
}
