#pragma once
// State machine for GPIO states of power distribution.
// Requires initialization of gpio, soft_timer and interrupts.

// The GPIO FSM has 6 states:
// - Idle
// - Emergency
// - Charge Preconfig
// - Charge
// - Drive Preconfig
// - Drive
//
// The preconfig states are used to configure GPIO events early in a sequence to disable unused
// pins. Primary events actually enable the pins required for that state.
#include <stdbool.h>

#include "chaos_config.h"
#include "event_queue.h"

// Initialize the FSM and sequences
void gpio_fsm_init(const ChaosConfig *cfg);

// Transition the GPIO FSM through a thin wrapper of fsm_process_event.
bool gpio_fsm_process_event(const Event *e);
