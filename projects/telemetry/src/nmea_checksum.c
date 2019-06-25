#include "nmea_checksum.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "log.h"

// Private method to convert hex char (stored in the 'h' parameter) to an int
// Store the result in the result pointer (which cannot be NULL)
// Returns STATUS_CODE_OK if the conversion was successful
static StatusCode prv_hex_to_int(char h, uint8_t *result) {
  if (result == NULL) {
    return status_msg(STATUS_CODE_INVALID_ARGS, "Cannot supply NULL pointer as arg\n");
  }
  if ('0' <= h && h <= '9') {
    *result = (uint8_t)(h - '0');
    return STATUS_CODE_OK;
  } else if ('A' <= h && h <= 'F') {
    // We only care about uppercase letters because the NMEA messages are all caps
    *result = (uint8_t)(h - 'A' + 10);
    return STATUS_CODE_OK;
  }
  // Just to make the checksum fail
  return status_msg(STATUS_CODE_INVALID_ARGS, "Invalid char supplied, cannot convert to hex\n");
}

static StatusCode prv_checksum_to_int(char tens, char ones, uint8_t *computed) {
  uint8_t int_tens = 0;
  uint8_t int_ones = 0;
  status_ok_or_return(prv_hex_to_int(tens, &int_tens));
  status_ok_or_return(prv_hex_to_int(ones, &int_ones));

  if (computed != NULL) {
    // Bitwise operation is equivalent to "int_tens * 16 + int_ones"
    // since both inputs are guaranteed to be [0, 15]
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
    LOG_DEBUG("first\n");
  }

  // We expect the last 2 characters in the message to be the sent checksum
  // Hence, the 3rd last character should be '*' (we subtract 3 instead of 2 to
  // account for \0)

  if (message[message_len - 5] != '*') {
    for(uint16_t i = 0; i <= message_len; i++){
      printf("%c", message[i]);
    }
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
