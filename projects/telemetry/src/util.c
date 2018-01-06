#include "util.h"
#include <stdbool.h>
#include <string.h>

static char s_hex[] = "0123456789ABCDEF";

char* prv_int_to_hex(uint8_t checksum) {
  // we know that it'll be a 2 digit hex number since the checksum is 8-bit
  static char return_value[3] = { 0 };
  return_value[0] = s_hex[checksum / 16];
  return_value[1] = s_hex[checksum % 16];
  return_value[2] = '\0';
  return return_value;
}

char* evm_gps_compute_checksum(char* message) {
  uint8_t sum = 0;
  uint8_t message_len = strlen(message);
  for (uint8_t i = 1; message[i] != '*' && i < message_len; i++) {
    sum ^= message[i];
  }
  return prv_int_to_hex(sum);
}

bool evm_gps_compare_checksum(char* message) {
  char* computed = evm_gps_compute_checksum(message);
  char* received = message + strlen(message) - 2;
  return strcmp(computed, received) == 0;
}
