#pragma once

typedef StatusCode (*SignalCallback)(Event e);

typedef StatusCode (*SignalSyncCallback)(void);

StatusCode signals_fsm_init(FSM *, BoardType, 
                SignalCallback, BlinkerDuration,
                uint8_t sync_frequency);

