#pragma once
// Chaos Power Path monitoring module.
// Requires gpio, gpio interrupts, adcs, soft timers and interrupts to be initialized.

#include <stdbool.h>
#include <stdint.h>

#include "adc.h"
#include "event_queue.h"
#include "gpio.h"
#include "soft_timer.h"
#include "status.h"

// Conversion function signature for ADCs.
typedef uint16_t (*PowerPathConversionFn)(uint16_t value);

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
  PowerPathConversionFn current_convert_fn;
  PowerPathConversionFn voltage_convert_fn;
  uint32_t period_millis;
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

// Starts monitoring the specified power source periodically.
StatusCode power_path_source_monitor_enable(PowerPathSource *source, uint32_t period_millis);

// Stops monitoring the specified power source periodically.
StatusCode power_path_source_monitor_disable(PowerPathSource *source);

// Reads the latest voltage and current from the specified power source.
StatusCode power_path_read_source(const PowerPathSource *source, PowerPathVCReadings *values);

// A function to update the power path based on an event rather than a direct function call, meant
// to emulate fsm_process_event with additional args.
bool power_path_process_event(PowerPathCfg *cfg, const Event *e);
