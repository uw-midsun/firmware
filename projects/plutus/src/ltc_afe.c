#include "ltc_afe.h"
#include "critical_section.h"
#include "ltc_afe_fsm.h"
#include "ltc_afe_impl.h"
#include "plutus_event.h"

#include <string.h>
#include "can_transmit.h"
#include "can_unpack.h"
#include "can.h"
#include "soft_timer.h"
#include "log.h"

static void prv_periodic_discharge_bitset(SoftTimerId timer_id, void *context) {
  LtcAfeStorage *afe = context;
  uint64_t data = { 0 };
  memcpy(&data, afe->discharge_bitset, sizeof(data));
  CAN_TRANSMIT_DISCHARGE_STATE(data);

  soft_timer_start_millis(PLUTUS_CFG_LTC_AFE_DISCHARGE_DUMP_PERIOD_MS, prv_periodic_discharge_bitset, afe,
                   NULL);
}

 static StatusCode prv_handle_can_bitset(const CanMessage *msg, void *context,
                                        CanAckStatus *ack_reply) {
  LtcAfeStorage *afe = context; 
  uint8_t module = 0;
  uint8_t discharge = 0; 

  CAN_UNPACK_SET_MODULE_DISCHARGE(msg, &module, &discharge);
  ltc_afe_impl_toggle_cell_discharge(afe, module, discharge); 
  return STATUS_CODE_OK;
}

StatusCode ltc_afe_init(LtcAfeStorage *afe, const LtcAfeSettings *settings) {
  status_ok_or_return(ltc_afe_impl_init(afe, settings));
  status_ok_or_return(ltc_afe_fsm_init(&afe->fsm, afe));

  status_ok_or_return(
    can_register_rx_handler(SYSTEM_CAN_MESSAGE_SET_MODULE_DISCHARGE, prv_handle_can_bitset, afe)
  );

  return soft_timer_start_millis(PLUTUS_CFG_LTC_AFE_DISCHARGE_DUMP_PERIOD_MS,
                          prv_periodic_discharge_bitset, afe, NULL);
}

StatusCode ltc_afe_set_result_cbs(LtcAfeStorage *afe, LtcAfeResultCallback cell_result_cb,
                                  LtcAfeResultCallback aux_result_cb, void *context) {
  bool disabled = critical_section_start();
  afe->cell_result_cb = cell_result_cb;
  afe->aux_result_cb = aux_result_cb;
  afe->result_context = context;
  critical_section_end(disabled);

  return STATUS_CODE_OK;
}

StatusCode ltc_afe_request_cell_conversion(LtcAfeStorage *afe) {
  return event_raise(PLUTUS_EVENT_AFE_TRIGGER_CELL_CONV, 0);
}

StatusCode ltc_afe_request_aux_conversion(LtcAfeStorage *afe) {
  return event_raise(PLUTUS_EVENT_AFE_TRIGGER_AUX_CONV, 0);
}

bool ltc_afe_process_event(LtcAfeStorage *afe, const Event *e) {
  return fsm_process_event(&afe->fsm, e);
}

StatusCode ltc_afe_toggle_cell_discharge(LtcAfeStorage *afe, uint16_t cell, bool discharge) {
  return ltc_afe_impl_toggle_cell_discharge(afe, cell, discharge);
}
