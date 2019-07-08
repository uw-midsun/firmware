#pragma once
// Module for indicating when the Strobe Light is turned on in the case of a
// BPS fault that the BMS detects. The Center Console board needs to provide
// the Driver with an illuminated dash indication of a BPS fault, to warn that
// the Main Power Switch is opening.
//
// In order to consolidate the logic, we use the Strobe Light CAN control
// message, which is controlled by Driver Controls: Pedal. Otherwise, we can
// duplicate the logic and process the BMS Heartbeat Messages, like the Driver
// Controls Master board does.
//
// Expects soft_timer, can, interrupts to be enabled.
#include "status.h"

// Configures the BPS Fault Dash Indicator handler.
StatusCode bps_indicator_init(void);
