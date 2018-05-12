#pragma once
// # Details about the NMEA checksum
// A given NMEA message starts with $, then contains various comma-seperated values,
// followed by a *, and then the message checksum (which is a 2-digit hex number).
// This header will help verify this checksum and the message integrity.

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include "status.h"

// Computes the checksum for a given NMEA message.
// Starts at the second characer of `message`,
// and continues until the `*` character, or until `message_len` characters
// are processed, whichever comes first.
StatusCode nmea_checksum_compute(char *message, size_t message_len, uint8_t *checksum);

// Computes the checksum for a given NMEA message, and compares the computed
// checksum against the checksum in the message.
// * message_len is byte size of message
// * Returns false if the given message does not have a checksum included or if the
//   checksums do not match.
bool nmea_checksum_validate(char *message, size_t message_len);
