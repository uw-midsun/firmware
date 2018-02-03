#include "nmea.h"
#include <stdbool.h>
#include <string.h>

static char s_hex[] = "0123456789ABCDEF";

// out must be a pointer to at least 3 chars
void prv_int_to_hex(uint8_t checksum, char *out) {
  // we know that it'll be a 2 digit hex number since the checksum is 8-bit
  out[0] = s_hex[checksum / 16];
  out[1] = s_hex[checksum % 16];
  out[2] = '\0';
}

void nmea_compute_checksum(char *message, size_t message_len, char *out) {
  uint8_t sum = 0;
  for (uint8_t i = 1; message[i] != '*' && i < message_len; i++) {
    sum ^= message[i];
  }
  prv_int_to_hex(sum, out);
}

bool nmea_compare_checksum(char *message, size_t message_len) {
  char *received = message + message_len - 2;
  if (*(received - 1) != '*') {
    // return false if there's no checksum in the message
    return false;
  }

  char computed[3];
  nmea_compute_checksum(message, message_len, computed);
  return strncmp(computed, received, 2) == 0;
}
