#pragma once
// State machine for GPIO states of power distribution.
// Requires initialization of gpio, soft_timer and interrupts.

#include <stdbool.h>

#include "chaos_config.h"
#include "event_queue.h"

// Initialize the FSM event.
void gpio_fsm_init(const ChaosConfig *cfg);

// transition the GPIO FSM through a thin wrapper of fsm_process_event.
bool gpio_fsm_process_event(const Event *e);
