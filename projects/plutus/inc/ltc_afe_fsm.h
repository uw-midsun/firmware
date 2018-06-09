#pragma once
// Wraps the LTC AFE module and handles all the sequencing.
// Requires LTC AFE, soft timers to be initialized.
//
#include "fsm.h"
#include "ltc_afe.h"

#define LTC_AFE_FSM_CELL_CONV_DELAY_MS 10
#define LTC_AFE_FSM_AUX_CONV_DELAY_MS 6

StatusCode ltc_afe_fsm_init(FSM *fsm, LtcAfeStorage *afe);
