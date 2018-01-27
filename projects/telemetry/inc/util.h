#pragma once
#include <stdbool.h>
#include <stdint.h>

// Computes the checksum for a given NMEA message.
//
// Starts at the second characer of `message`,
// and continues until the `*` character.
//
// `out` must be a pointer to at least 3 `char`s.
void evm_gps_compute_checksum(char *message, char *out);

// Computes the checksum for a given NMEA message, and compares the computed
// checksum against the checksum in the message.
//
// Returns false if the given message does not have a checksum included.
bool evm_gps_compare_checksum(char *message);
