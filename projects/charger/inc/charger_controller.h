#pragma once

#include <stdbool.h>
#include <stdint.h>

#include "can_ack.h"
#include "soft_timer.h"

typedef struct ChargerSettings {
  uint16_t max_voltage;
  uint16_t max_current;
  SoftTimerID timer_id;
} ChargerSettings;

void charger_init(ChargerSettings *settings);

void charger_set_state(bool active);
