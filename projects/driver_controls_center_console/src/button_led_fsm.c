#include "button_led_fsm.h"

#include "center_console_event.h"
#include "fsm.h"
#include "gpio.h"

#include "log.h"

// Contains all the context data needed for the callback
typedef struct ButtonFsmCtx {
  // The Button ID that allows us to reference it
  EECenterConsoleDigitalInput button_id;
  // Pin for the LED on the GPIO Expander
  GpioExpanderPin pin;
  GpioExpanderStorage *expander_storage;
} ButtonFsmCtx;

static ButtonFsmCtx s_fsm_ctxs[NUM_CENTER_CONSOLE_BUTTON_LEDS];

// Select whether the FSM is configured to handle that specific LED. The Event
// data field should be the ID of the FSM that needs to be transitioned.
static bool prv_guard_select_button(const Fsm *fsm, const Event *e, void *context) {
  ButtonFsmCtx *fsm_ctx = context;

  return e->data == fsm_ctx->button_id;
}

FSM_DECLARE_STATE(button_led_on);
FSM_DECLARE_STATE(button_led_off);

FSM_STATE_TRANSITION(button_led_on) {
  FSM_ADD_GUARDED_TRANSITION(CENTER_CONSOLE_EVENT_BUTTON_SET_STATE_OFF, prv_guard_select_button,
                             button_led_off);
}

FSM_STATE_TRANSITION(button_led_off) {
  FSM_ADD_GUARDED_TRANSITION(CENTER_CONSOLE_EVENT_BUTTON_SET_STATE_ON, prv_guard_select_button,
                             button_led_on);
}

static void prv_button_led_on(Fsm *fsm, const Event *e, void *context) {
  ButtonFsmCtx *button_fsm_ctx = context;

  gpio_expander_set_state(button_fsm_ctx->expander_storage, button_fsm_ctx->pin, GPIO_STATE_HIGH);
}

static void prv_button_led_off(Fsm *fsm, const Event *e, void *context) {
  ButtonFsmCtx *button_fsm_ctx = context;

  gpio_expander_set_state(button_fsm_ctx->expander_storage, button_fsm_ctx->pin, GPIO_STATE_LOW);
}

void button_led_fsm_init(void) {
  fsm_state_init(button_led_on, prv_button_led_on);
  fsm_state_init(button_led_off, prv_button_led_off);
}

StatusCode button_led_fsm_create(Fsm *fsm, GpioExpanderStorage *expander_storage,
                                 EECenterConsoleDigitalInput button_id, GpioExpanderPin pin,
                                 const char *fsm_name) {
  if (button_id > NUM_EE_CENTER_CONSOLE_DIGITAL_INPUTS) {
    return status_code(STATUS_CODE_RESOURCE_EXHAUSTED);
  }

  s_fsm_ctxs[button_id].button_id = button_id;
  s_fsm_ctxs[button_id].pin = pin;
  s_fsm_ctxs[button_id].expander_storage = expander_storage;

  // Start with all buttons with low output
  const GpioSettings output_settings = {
    .direction = GPIO_DIR_OUT,  //
    .state = GPIO_STATE_LOW,    //
  };
  status_ok_or_return(gpio_expander_init_pin(expander_storage, pin, &output_settings));

  // Start in the off state
  fsm_init(fsm, fsm_name, &button_led_off, &s_fsm_ctxs[button_id]);

  return STATUS_CODE_OK;
}
