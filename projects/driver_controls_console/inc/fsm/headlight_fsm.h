#pragma once
// Keeps track of whether the headlights are on
#include "event_arbiter.h"
#include "fsm.h"

StatusCode headlight_fsm_init(Fsm *fsm, EventArbiterStorage *storage);
