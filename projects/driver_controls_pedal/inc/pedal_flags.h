#pragma once

// This flag controls whether the Drive Output messages contain Throttle
// position when in the brake zone. If unset, the default behaviour is to
// coast when in the brake zone.
//
// In order to not need to re-flash every time, we can potentially tie this to
// one of the unused Stalk Inputs instead of this #define workaround. In
// addition, logic should probably be added either in Driver Controls: Pedal
// or Motor Controller interface to disable regenerative braking when:
//
//    1. Driving below certain speeds
//    2. Battery SOC is above a predetermined threshold
//    3. If the driver doesn't want regen braking for some reason
//
#define PEDAL_FLAG_ENABLE_REGEN_BRAKING
