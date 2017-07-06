#pragma once
#include "event_queue.h"
#include "fsm_state.h"
#include "status.h"

typedef void (*DriverFSMInit)(FSM *power_fsm, void *context);

void driver_state_init(FSMGroup *fsm_group);

StatusCode driver_state_add_fsm(DriverFSMInit driver_fsm_init);

char* driver_state_get_fsm(uint8_t index);

bool driver_state_process_event(Event *e);
