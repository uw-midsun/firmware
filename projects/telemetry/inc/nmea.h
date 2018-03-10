#pragma once
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "status.h"

// Computes the checksum for a given NMEA message.
//
// Starts at the second characer of `message`,
// and continues until the `*` character, or until `message_len` characters
// are processed, whichever comes first.
//
// `out` must be a pointer to at least 3 `char`s.
StatusCode nmea_compute_checksum(char *message, size_t message_len, char *out, size_t out_len);

// Computes the checksum for a given NMEA message, and compares the computed
// checksum against the checksum in the message.
//
// Returns false if the given message does not have a checksum included.
bool nmea_compare_checksum(char *message, size_t message_len);
