#pragma once
// System config/setup for Plutus
//
// Initializes all board modules for the given board type
//
// Master:
// * CAN device ID: BPS Master
// * Sequenced relays: Main battery
// * BPS Heartbeat TX
// * Powertrain Heartbeat RX
// * AFE + fault monitoring
// * ADC + fault monitoring
// * Killswitch monitoring
//
// Slave:
// * CAN device ID: BPS Slave
// * Sequenced relays: Slave battery
// * BPS Heartbeat RX
// * Killswitch bypass
#include "bps_heartbeat.h"
#include "can.h"
#include "heartbeat_rx.h"
#include "killswitch.h"
#include "ltc_adc.h"
#include "ltc_afe.h"
#include "sequenced_relay.h"

typedef enum {
  PLUTUS_SYS_TYPE_MASTER = 0,
  PLUTUS_SYS_TYPE_SLAVE,
  NUM_PLUTUS_SYS_TYPES,
} PlutusSysType;

typedef struct PlutusSysStorage {
  CANStorage can;
  LtcAfeStorage ltc_afe;
  LtcAdcStorage ltc_adc;
  BpsHeartbeatStorage bps_heartbeat;
  SequencedRelayStorage relay;

  HeartbeatRxHandlerStorage powertrain_heartbeat_handler;
  HeartbeatRxHandlerStorage bps_heartbeat_handler;
  KillswitchStorage killswitch;

  PlutusSysType type;
} PlutusSysStorage;

// Loads the configured type
PlutusSysType plutus_sys_get_type(void);

// Initializes all modules associated with the system type
StatusCode plutus_sys_init(PlutusSysStorage *storage, PlutusSysType type);
