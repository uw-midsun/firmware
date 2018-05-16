#include "nmea_checksum.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static const int HEX_ERROR = 16;

// Private method to convert hex char to int
static uint8_t prv_hex_to_int(char h) {
  if ('0' <= h && h <= '9') {
    return h - '0';
  } else if ('A' <= h && h <= 'F') {
    // We only care about uppercase because the NMEA messages are all caps
    return h - 'A' + 10;
  }
  // Just to make the checksum fail
  return HEX_ERROR;
}

static StatusCode prv_checksum_to_int(char tens, char ones, uint8_t *computed) {
  uint8_t int_tens = prv_hex_to_int(tens);
  uint8_t int_ones = prv_hex_to_int(ones);

  // Checks for the 16 because it is the result sent in case of error
  if (ones == HEX_ERROR || tens == HEX_ERROR) {
    return STATUS_CODE_INVALID_ARGS;
  }
  if (computed != NULL) {
    // Bitwise operation is equivilent to "int_tens * 16 + int_ones" with both
    // variables truncated to 4 least significant bits
    *computed = ((int_tens & 0xF) << 4) | (int_ones & 0xF);
  }
  return STATUS_CODE_OK;
}

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
  if (message == NULL || message_len < 4) {
    return false;
  }

  // We expect the last 2 characters in the message to be the sent checksum
  // Hence, the 3rd last character should be '*' (we subtract 3 instead of 2 to
  // account for \0)

  if (message[message_len - 3] != '*') {
    // return false if there's no checksum in the message
    return false;
  }

  // Extracts the received checksum
  uint8_t received_checksum = 0;
  StatusCode status_extracted =
      prv_checksum_to_int(message[message_len - 2], message[message_len - 1], &received_checksum);

  // Computes the checksum
  uint8_t computed = 0;
  StatusCode status_computed = nmea_checksum_compute(message, message_len, &computed);
  if (!status_ok(status_computed) || !status_ok(status_extracted)) {
    // return false if nmea_compute_checksum fails
    return false;
  }
  return computed == received_checksum;
}
