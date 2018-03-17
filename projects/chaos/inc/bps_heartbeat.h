#pragma once

#include "status.h"

// Once invoked this assumes that the BPS will always remain active and sending heartbeats!
StatusCode bps_heartbeat_start(void);
