#include "ltc_afe_fsm.h"
#include "ltc_afe_impl.h"
#include "plutus_event.h"
#include "soft_timer.h"

FSM_DECLARE_STATE(afe_idle);
FSM_DECLARE_STATE(afe_trigger_cell_conv);
FSM_DECLARE_STATE(afe_read_cells);
FSM_DECLARE_STATE(afe_trigger_aux_conv);
FSM_DECLARE_STATE(afe_read_aux);
FSM_DECLARE_STATE(afe_aux_complete);

static bool prv_all_aux_complete(const struct FSM *fsm, const Event *e, void *context) {
  return e->data >= 12;
}

FSM_STATE_TRANSITION(afe_idle) {
  FSM_ADD_TRANSITION(PLUTUS_EVENT_AFE_TRIGGER_CELL_CONV, afe_trigger_cell_conv);
  FSM_ADD_TRANSITION(PLUTUS_EVENT_AFE_TRIGGER_AUX_CONV, afe_trigger_aux_conv);
}

FSM_STATE_TRANSITION(afe_trigger_cell_conv) {
  FSM_ADD_TRANSITION(PLUTUS_EVENT_AFE_CELL_CONV_COMPLETE, afe_read_cells);
}

FSM_STATE_TRANSITION(afe_read_cells) {
  FSM_ADD_TRANSITION(PLUTUS_EVENT_AFE_CALLBACK_RUN, afe_idle);
}

FSM_STATE_TRANSITION(afe_trigger_aux_conv) {
  FSM_ADD_TRANSITION(PLUTUS_EVENT_AFE_AUX_CONV_COMPLETE, afe_read_aux);
}

FSM_STATE_TRANSITION(afe_read_aux) {
  FSM_ADD_GUARDED_TRANSITION(PLUTUS_EVENT_AFE_TRIGGER_AUX_CONV, prv_all_aux_complete,
                             afe_aux_complete);
  FSM_ADD_TRANSITION(PLUTUS_EVENT_AFE_TRIGGER_AUX_CONV, afe_trigger_aux_conv);
}

FSM_STATE_TRANSITION(afe_aux_complete) {
  FSM_ADD_TRANSITION(PLUTUS_EVENT_AFE_CALLBACK_RUN, afe_idle);
}

static void prv_cell_conv_timeout(SoftTimerID timer_id, void *context) {
  event_raise(PLUTUS_EVENT_AFE_CELL_CONV_COMPLETE, 0);
}

static void prv_aux_conv_timeout(SoftTimerID timer_id, void *context) {
  uint32_t device_cell = (uint32_t)context;
  event_raise(PLUTUS_EVENT_AFE_AUX_CONV_COMPLETE, device_cell);
}

static void prv_afe_trigger_cell_conv_output(struct FSM *fsm, const Event *e, void *context) {
  LtcAfeStorage *afe = context;
  ltc_afe_impl_trigger_cell_conv(afe);

  soft_timer_start_millis(LTC_AFE_FSM_CELL_CONV_DELAY_MS, prv_cell_conv_timeout, NULL, NULL);
}

static void prv_afe_read_cells_output(struct FSM *fsm, const Event *e, void *context) {
  LtcAfeStorage *afe = context;

  ltc_afe_impl_read_cells(afe);

  // Raise the event first in case the user raises a trigger conversion event in the callback
  event_raise(PLUTUS_EVENT_AFE_CALLBACK_RUN, 0);

  if (afe->cell_result_cb != NULL) {
    afe->cell_result_cb(afe->cell_voltages, PLUTUS_CFG_AFE_TOTAL_CELLS, afe->result_context);
  }
}

static void prv_afe_trigger_aux_conv_output(struct FSM *fsm, const Event *e, void *context) {
  LtcAfeStorage *afe = context;
  uint32_t device_cell = e->data;
  ltc_afe_impl_trigger_aux_conv(afe, device_cell);

  soft_timer_start_millis(LTC_AFE_FSM_AUX_CONV_DELAY_MS, prv_aux_conv_timeout, (void *)device_cell,
                          NULL);
}

static void prv_afe_read_aux_output(struct FSM *fsm, const Event *e, void *context) {
  LtcAfeStorage *afe = context;
  uint16_t device_cell = e->data;

  ltc_afe_impl_read_aux(afe, device_cell);

  // Kick-off the next aux conversion
  event_raise(PLUTUS_EVENT_AFE_TRIGGER_AUX_CONV, device_cell + 1);
}

static void prv_afe_aux_complete_output(struct FSM *fsm, const Event *e, void *context) {
  LtcAfeStorage *afe = context;

  // Raise the event first in case the user raises a trigger conversion event in the callback
  event_raise(PLUTUS_EVENT_AFE_CALLBACK_RUN, 0);

  // 12 aux conversions complete - the array should be fully populated
  if (afe->aux_result_cb != NULL) {
    afe->aux_result_cb(afe->aux_voltages, PLUTUS_CFG_AFE_TOTAL_CELLS, afe->result_context);
  }
}

StatusCode ltc_afe_fsm_init(FSM *fsm, LtcAfeStorage *afe) {
  fsm_state_init(afe_idle, NULL);
  fsm_state_init(afe_trigger_cell_conv, prv_afe_trigger_cell_conv_output);
  fsm_state_init(afe_read_cells, prv_afe_read_cells_output);
  fsm_state_init(afe_trigger_aux_conv, prv_afe_trigger_aux_conv_output);
  fsm_state_init(afe_read_aux, prv_afe_read_aux_output);
  fsm_state_init(afe_aux_complete, prv_afe_aux_complete_output);

  fsm_init(fsm, "LTC AFE FSM", &afe_idle, afe);

  return STATUS_CODE_OK;
}
