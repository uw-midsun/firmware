#pragma once
// Module for interfacing 
// Requires ADS1015, event queue to be initialized
// Monitors ADS1015 over I2C and raises events on input state change.

#include <stdio.h>
#include <stddef.h>
#include <stdbool.h>
#include "ads1015.h"
#include "ads1015_def.h"
//#include "solar_master_relay.h"
#include "solar_master_event.h"
#include "exported_enums.h"
#include "event_queue.h"
#include "log.h"

#define SOLAR_MASTER_CURRENT_CHANNEL 0

// TODO: Adjust by temperature??
#define SOLAR_MASTER_CURRENT_GRADIENT 264
#define SOLAR_MASTER_CURRENT_INTERCEPT 336

#define SOLAR_MASTER_CURRENT_I2C_BUS_PORT I2C_PORT_1
#define SOLAR_MASTER_CURRENT_ADC_ADDR 0 // ?????

#define SOLAR_MASTER_CURRENT_SAMPLE_SIZE 100

#define CURRENT_ADC_READY_PIN \
  { GPIO_PORT_A, 4 }

typedef struct SolarMasterCurrent {
  Ads1015Storage *ads1015;
  int16_t averaging[SOLAR_MASTER_CURRENT_SAMPLE_SIZE];
  int16_t lastAverage;
  uint16_t counter;
  uint16_t zero_point;
  bool calibrated;
} SolarMasterCurrent;

// Registers callbacks for analog inputs. |ads1015| should be initialized.
StatusCode solar_master_current_init(SolarMasterCurrent *current, Ads1015Storage *ads1015);
