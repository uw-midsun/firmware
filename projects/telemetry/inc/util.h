#pragma once
#include <stdbool.h>
#include <stdint.h>

// Computes the checksum for the given message.
// The method takes a NMEA message, it automatically stops at *
// out must be a pointer to at least 3 chars
void evm_gps_compute_checksum(char *message, char *out);

// computes the checksum for the message, and compares that against
// the checksum in the message
// returns false if the message does not have a checksum at the end
bool evm_gps_compare_checksum(char *message);
