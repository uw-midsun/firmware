#pragma once
// Chaos Power Path monitoring module.
// Requires gpio, gpio interrupts, adcs, soft timers and interrupts to be initialized.

#include <stdbool.h>
#include <stdint.h>

#include "adc.h"
#include "gpio.h"
#include "soft_timer.h"
#include "status.h"

// Conversion callbacks for ADCs.
typedef uint16_t (*PowerPathConversionCallback)(uint16_t value);

typedef enum {
  POWER_PATH_SOURCE_ID_DCDC = 0,
  POWER_PATH_SOURCE_ID_AUX_BAT,
  NUM_POWER_PATH_SOURCE_IDS,
} PowerPathSourceID;

// For storing voltage and current values.
typedef struct PowerPathVCReadings {
  uint16_t voltage;
  uint16_t current;
} PowerPathVCReadings;

// For storing the Power Path Source (eg AUX Battery vs DCDCs).
typedef struct PowerPathSource {
  const PowerPathSourceID id;
  const GPIOAddress uv_ov_pin;
  const GPIOAddress voltage_pin;
  const GPIOAddress current_pin;
  volatile PowerPathVCReadings readings;
  PowerPathConversionCallback current_convert;
  PowerPathConversionCallback voltage_convert;
  uint32_t period_us;
  SoftTimerID timer_id;
  bool monitoring_active;
} PowerPathSource;

// For storing the power path configuration.
typedef struct PowerPathCfg {
  const GPIOAddress enable_pin;
  const GPIOAddress shutdown_pin;
  PowerPathSource aux_bat;
  PowerPathSource dcdc;
} PowerPathCfg;

// Configures the GPIO pins for the power path.
StatusCode power_path_init(PowerPathCfg *pp);

// Enables the power path.
StatusCode power_path_enable(const PowerPathCfg *pp);

// Disables the power path, also disables monitoring on both sources.
StatusCode power_path_disable(PowerPathCfg *pp);

// Starts monitoring the specified power source periodically.
StatusCode power_path_source_monitor_enable(PowerPathSource *source, uint32_t period_us);

// Stops monitoring the specified power source periodically.
StatusCode power_path_source_monitor_disable(PowerPathSource *source);

// Reads the latest voltage and current from the specified power source.
StatusCode power_path_read_source(const PowerPathSource *source, PowerPathVCReadings *values);
