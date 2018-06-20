#pragma once
// Monitor voltage/current/temperature for faults
// Requires LTC AFE, LTC ADC, BPS heartbeat to be initialized
#include "bps_heartbeat.h"
#include "ltc_adc.h"
#include "ltc_afe.h"

typedef struct FaultMonitorSettings {
  LtcAfeStorage *ltc_afe;
  LtcAdcStorage *ltc_adc;
  BpsHeartbeatStorage *bps_heartbeat;

  // In 100uV (0.1mV)
  uint16_t overvoltage;
  uint16_t undervoltage;

  // In mC
  uint16_t overtemp_charge;
  uint16_t overtemp_discharge;

  // In mA
  int32_t overcurrent_charge;
  int32_t overcurrent_discharge;
} FaultMonitorSettings;

typedef struct FaultMonitorResult {
  uint16_t cell_voltages[PLUTUS_CFG_AFE_TOTAL_CELLS];
  uint16_t temp_voltages[PLUTUS_CFG_AFE_TOTAL_CELLS];
  int32_t current;
} FaultMonitorResult;

typedef struct FaultMonitorStorage {
  FaultMonitorSettings settings;
  FaultMonitorResult result;
} FaultMonitorStorage;

// |storage| should persist. |settings.ltc_afe| and |settings.bps_heartbeat| should be initialized.
StatusCode fault_monitor_init(FaultMonitorStorage *storage, const FaultMonitorSettings *settings);

// Processes fault events
bool fault_monitor_process_event(FaultMonitorStorage *storage, const Event *e);
