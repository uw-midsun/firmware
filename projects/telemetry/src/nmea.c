#include "nmea.h"
#include <stdbool.h>
#include <string.h>

static char s_hex[] = "0123456789ABCDEF";

// Out must be a pointer to at least 3 chars
void prv_int_to_hex(uint8_t checksum, char *out) {
  // We know that it'll be a 2 digit hex number since the checksum is 8-bit
  out[0] = s_hex[checksum >> 0x4];
  out[1] = s_hex[checksum & 0xF];
  out[2] = '\0';
}


// The checksum is calculated by taking the XOR of all characters between
// (but not including) the '$' and '*' character
// An example NMEA message is "$GPGLL,2503.6319,N,12136.0099,E,053740.000,A,A*52"
// In the message above, the checksum would be 52.
StatusCode nmea_compute_checksum(char *message, size_t message_len, char *out, size_t out_len) {
  if (out_len < 3) {
    return STATUS_CODE_INVALID_ARGS;
  }
  uint8_t sum = 0;
  // start at the 2nd character, since the first character is $
  for (uint8_t i = 1; message[i] != '*' && i < (message_len - 1); i++) {
    sum ^= message[i];
  }
  prv_int_to_hex(sum, out);
  return STATUS_CODE_OK;
}

bool nmea_compare_checksum(char *message, size_t message_len) {
  // We expect the last 2 characters in the message to be the sent checksum
  // Hence, the 3rd last character should be '*' (we subtract 3 instead of 2 to
  // account for \0)
  if (message_len < 3) {
    return false;
  }
  char *received = message + message_len - 3;
  if (*(received - 1) != '*') {
    // return false if there's no checksum in the message
    return false;
  }

  char computed[3];
  StatusCode status = nmea_compute_checksum(message, message_len, computed, sizeof(computed));
  if (status != STATUS_CODE_OK) {
    // return false if nmea_compute_checksum fails
    return false;
  }
  return strncmp(computed, received, 2) == 0;
}
