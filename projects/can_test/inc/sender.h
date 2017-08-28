#pragma once
#include <stdint.h>

// Sets up a repeating timer that essentially attempts to flood the CAN bus
// Also periodically prints the number of transmitted packets per second
void sender_init(uint16_t msg_id);
