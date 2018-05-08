// # The NMEA message format
// A given NMEA message starts with $, then contains various comma-seperated values,
// followed by a *, and then the message checksum (which is a 2-digit hex number).
// This header will aid in parsing the NMEA messages sent by the GPS.

#pragma once
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include "status.h"

// Computes the checksum for a given NMEA message.
// Starts at the second characer of `message`,
// and continues until the `*` character, or until `message_len` characters
// are processed, whichever comes first.
// * message_len is byte size of message
// `out` must be a pointer to at least 3 `char`s.
StatusCode nmea_compute_checksum(char *message, size_t message_len, char *out, size_t out_len);

// Computes the checksum for a given NMEA message, and compares the computed
// checksum against the checksum in the message.
// * message_len is byte size of message
// * Returns false if the given message does not have a checksum included or if the
//   checksums do not match.
bool nmea_compare_checksum(char *message, size_t message_len);
