#pragma once
// Monitor voltage/current/temperature for faults
// Requires LTC AFE, current sense, BPS heartbeat to be initialized
#include "bps_heartbeat.h"
#include "current_sense.h"
#include "ltc_afe.h"
#include "thermistor.h"

typedef struct FaultMonitorSettings {
  LtcAfeStorage *ltc_afe;
  CurrentSenseStorage *current_sense;
  BpsHeartbeatStorage *bps_heartbeat;

  // In 100uV (0.1mV)
  uint16_t overvoltage;
  uint16_t undervoltage;

  // In mC
  uint16_t overtemp_charge;
  uint16_t overtemp_discharge;

  // In mA - all values should be positive
  int32_t overcurrent_charge;
  int32_t overcurrent_discharge;
  // Minimum charge current before we consider it as charging
  int32_t charge_current_deadzone;
} FaultMonitorSettings;

typedef struct FaultMonitorResult {
  uint16_t cell_voltages[PLUTUS_CFG_AFE_TOTAL_CELLS];
  uint16_t temp_voltages[PLUTUS_CFG_AFE_TOTAL_CELLS];
  uint32_t total_voltage;
  int32_t current;
  bool charging;
} FaultMonitorResult;

typedef struct FaultMonitorStorage {
  FaultMonitorSettings settings;
  FaultMonitorResult result;
  size_t num_afe_fsm_faults;
  uint8_t cell_faults[PLUTUS_CFG_AFE_TOTAL_CELLS];
  uint8_t temp_faults[PLUTUS_CFG_AFE_TOTAL_CELLS];

  // in uA
  int32_t charge_current_limit;
  int32_t discharge_current_limit;
  int32_t min_charge_current;

  // in 100uV - node voltage of thermistors
  uint16_t discharge_temp_node_limit;
  uint16_t charge_temp_node_limit;
} FaultMonitorStorage;

// |storage| should persist. |settings.ltc_afe| and |settings.bps_heartbeat| should be initialized.
StatusCode fault_monitor_init(FaultMonitorStorage *storage, const FaultMonitorSettings *settings);

// Processes fault events
bool fault_monitor_process_event(FaultMonitorStorage *storage, const Event *e);
