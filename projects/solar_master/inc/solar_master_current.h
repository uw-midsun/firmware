#pragma once
// Module for interfacing with the ADS1015, reading values from the hall effect sensor.
// Requires ADS1015 to be initialized

#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include "ads1015.h"
#include "ads1015_def.h"
#include "event_queue.h"
#include "exported_enums.h"
#include "log.h"
#include "solar_master_event.h"

#define SOLAR_MASTER_CURRENT_CHANNEL 0

#define SOLAR_MASTER_CURRENT_GRADIENT 264
#define SOLAR_MASTER_CURRENT_INTERCEPT 336

#define SOLAR_MASTER_CURRENT_ADC_ADDR 0

#define SOLAR_MASTER_CURRENT_SAMPLE_SIZE 100

#define CURRENT_ADC_READY_PIN \
  { GPIO_PORT_A, 4 }

typedef struct SolarMasterCurrent {
  Ads1015Storage *ads1015;
  int16_t averaging[SOLAR_MASTER_CURRENT_SAMPLE_SIZE];
  int16_t sliding_sum;
  uint16_t counter;
  uint16_t zero_point;
} SolarMasterCurrent;

// Registers callbacks for analog inputs. |ads1015| should be initialized.
StatusCode solar_master_current_init(SolarMasterCurrent *current, Ads1015Storage *ads1015);
