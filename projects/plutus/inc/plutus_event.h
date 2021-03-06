#pragma once

typedef enum {
  PLUTUS_EVENT_CAN_TX = 0,
  PLUTUS_EVENT_CAN_RX,
  PLUTUS_EVENT_CAN_FAULT,

  PLUTUS_EVENT_AFE_TRIGGER_CELL_CONV,
  PLUTUS_EVENT_AFE_CELL_CONV_COMPLETE,
  PLUTUS_EVENT_AFE_TRIGGER_AUX_CONV,
  PLUTUS_EVENT_AFE_AUX_CONV_COMPLETE,
  PLUTUS_EVENT_AFE_CALLBACK_RUN,
  PLUTUS_EVENT_AFE_FAULT,
} PlutusEvent;
