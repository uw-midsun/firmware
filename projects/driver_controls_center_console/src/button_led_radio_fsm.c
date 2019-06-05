#include "button_led_radio_fsm.h"

#include "exported_enums.h"
#include "gpio.h"
#include "gpio_expander.h"
#include "input_event.h"

#include "log.h"
typedef struct RadioButtonFsmCtx {
  GpioExpanderStorage *expander_storage;

  GpioExpanderPin drive_pin;
  GpioExpanderPin neutral_pin;
  GpioExpanderPin reverse_pin;
} RadioButtonFsmCtx;

static RadioButtonFsmCtx s_fsm_ctxs;

static void prv_button_drive_on(Fsm *fsm, const Event *e, void *context) {
  // All LEDs should be off except for Drive
  RadioButtonFsmCtx *button_fsm_ctx = context;
  LOG_DEBUG("DRIVE\n");

  gpio_expander_set_state(button_fsm_ctx->expander_storage, button_fsm_ctx->neutral_pin,
                          GPIO_STATE_LOW);
  gpio_expander_set_state(button_fsm_ctx->expander_storage, button_fsm_ctx->reverse_pin,
                          GPIO_STATE_LOW);
  gpio_expander_set_state(button_fsm_ctx->expander_storage, button_fsm_ctx->drive_pin,
                          GPIO_STATE_LOW);

  gpio_expander_set_state(button_fsm_ctx->expander_storage, button_fsm_ctx->drive_pin,
                          GPIO_STATE_HIGH);
  LOG_DEBUG("DRIVE: Done\n");
}

static void prv_button_neutral_on(Fsm *fsm, const Event *e, void *context) {
  // All LEDs should be off except for Neutral
  RadioButtonFsmCtx *button_fsm_ctx = context;
  LOG_DEBUG("NEUTRAL: Start\n");

  gpio_expander_set_state(button_fsm_ctx->expander_storage, button_fsm_ctx->neutral_pin,
                          GPIO_STATE_LOW);
  gpio_expander_set_state(button_fsm_ctx->expander_storage, button_fsm_ctx->reverse_pin,
                          GPIO_STATE_LOW);
  gpio_expander_set_state(button_fsm_ctx->expander_storage, button_fsm_ctx->drive_pin,
                          GPIO_STATE_LOW);

  gpio_expander_set_state(button_fsm_ctx->expander_storage, button_fsm_ctx->neutral_pin,
                          GPIO_STATE_HIGH);
  LOG_DEBUG("NEUTRAL: Done\n");
}

static void prv_button_reverse_on(Fsm *fsm, const Event *e, void *context) {
  // Go through all pins and turn them all off
  RadioButtonFsmCtx *button_fsm_ctx = context;
  LOG_DEBUG("REVERSE: Start\n");

  gpio_expander_set_state(button_fsm_ctx->expander_storage, button_fsm_ctx->neutral_pin,
                          GPIO_STATE_LOW);
  gpio_expander_set_state(button_fsm_ctx->expander_storage, button_fsm_ctx->reverse_pin,
                          GPIO_STATE_LOW);
  gpio_expander_set_state(button_fsm_ctx->expander_storage, button_fsm_ctx->drive_pin,
                          GPIO_STATE_LOW);

  gpio_expander_set_state(button_fsm_ctx->expander_storage, button_fsm_ctx->reverse_pin,
                          GPIO_STATE_HIGH);
  LOG_DEBUG("REVERSE: Done\n");
}

FSM_DECLARE_STATE(button_group_neutral_on);
FSM_DECLARE_STATE(button_group_drive_on);
FSM_DECLARE_STATE(button_group_reverse_on);

FSM_STATE_TRANSITION(button_group_neutral_on) {
  FSM_ADD_TRANSITION(CENTER_CONSOLE_EVENT_DRIVE_OUTPUT_DIRECTION_DRIVE, button_group_drive_on);
  FSM_ADD_TRANSITION(CENTER_CONSOLE_EVENT_DRIVE_OUTPUT_DIRECTION_REVERSE, button_group_reverse_on);
}

FSM_STATE_TRANSITION(button_group_drive_on) {
  FSM_ADD_TRANSITION(CENTER_CONSOLE_EVENT_DRIVE_OUTPUT_DIRECTION_NEUTRAL, button_group_neutral_on);
  FSM_ADD_TRANSITION(CENTER_CONSOLE_EVENT_DRIVE_OUTPUT_DIRECTION_REVERSE, button_group_reverse_on);
}

FSM_STATE_TRANSITION(button_group_reverse_on) {
  FSM_ADD_TRANSITION(CENTER_CONSOLE_EVENT_DRIVE_OUTPUT_DIRECTION_DRIVE, button_group_drive_on);
  FSM_ADD_TRANSITION(CENTER_CONSOLE_EVENT_DRIVE_OUTPUT_DIRECTION_NEUTRAL, button_group_neutral_on);
}

void button_led_radio_fsm_init(void) {
  fsm_state_init(button_group_neutral_on, prv_button_neutral_on);
  fsm_state_init(button_group_drive_on, prv_button_drive_on);
  fsm_state_init(button_group_reverse_on, prv_button_reverse_on);
}

StatusCode button_led_radio_fsm_create(Fsm *fsm, GpioExpanderStorage *expander_storage,
                                       ButtonLedRadioSettings *settings, const char *fsm_name) {
  s_fsm_ctxs.expander_storage = expander_storage;
  s_fsm_ctxs.drive_pin = settings->drive_pin;
  s_fsm_ctxs.neutral_pin = settings->neutral_pin;
  s_fsm_ctxs.reverse_pin = settings->reverse_pin;

  fsm_init(fsm, fsm_name, &button_group_neutral_on, &s_fsm_ctxs);

  // Start with all buttons with low
  const GpioSettings output_settings = {
    .direction = GPIO_DIR_OUT,  //
    .state = GPIO_STATE_LOW,    //
  };
  status_ok_or_return(
      gpio_expander_init_pin(expander_storage, settings->drive_pin, &output_settings));
  status_ok_or_return(
      gpio_expander_init_pin(expander_storage, settings->neutral_pin, &output_settings));
  status_ok_or_return(
      gpio_expander_init_pin(expander_storage, settings->reverse_pin, &output_settings));

  return STATUS_CODE_OK;
}
