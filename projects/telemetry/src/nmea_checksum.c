#include "nmea_checksum.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// The checksum is calculated by taking the XOR of all characters between
// (but not including) the '$' and '*' character
// An example NMEA message is "$GPGLL,2503.6319,N,12136.0099,E,053740.000,A,A*52"
// In the message above, the checksum would be 52.
StatusCode nmea_checksum_compute(char *message, size_t message_len, uint8_t *checksum) {
  if (checksum == NULL || message == NULL || message_len < 4) {
    return status_msg(STATUS_CODE_INVALID_ARGS,
                      "out or message cannot be NULL. message_len must be greater than 3");
  }

  uint8_t sum = 0;
  // start at the 2nd character, since the first character is $
  for (uint8_t i = 1; message[i] != '*' && i < (message_len - 1); i++) {
    sum ^= message[i];
  }
  *checksum = sum;
  return STATUS_CODE_OK;
}

bool nmea_checksum_validate(char *message, size_t message_len) {
  if (message == NULL || message_len < 4 || message[message_len - 1] != '\0') {
    return false;
  }

  // We expect the last 2 characters in the message to be the sent checksum
  // Hence, the 3rd last character should be '*' (we subtract 3 instead of 2 to
  // account for \0)
  char *received = message + message_len - 3;

  if (*(received - 1) != '*') {
    // return false if there's no checksum in the message
    return false;
  }

  // Extracts the received checksum
  uint32_t received_checksum = (uint32_t)strtol(received, NULL, 16);

  // Computes the checksum
  uint8_t computed = 0;
  StatusCode status = nmea_checksum_compute(message, message_len, &computed);
  if (status != STATUS_CODE_OK) {
    // return false if nmea_compute_checksum fails
    return false;
  }
  return computed == received_checksum;
}
