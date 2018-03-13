#include "util.h"
#include <stdbool.h>
#include <string.h>
#include "log.h"

static char s_hex[] = "0123456789ABCDEF";

// out must be a pointer to at least 3 chars
void prv_int_to_hex(uint8_t checksum, char* out) {
  // we know that it'll be a 2 digit hex number since the checksum is 8-bit
  out[0] = s_hex[checksum / 16];
  out[1] = s_hex[checksum % 16];
  out[2] = '\0';
}

// out must be a pointer to at least 3 chars
void evm_gps_compute_checksum(char *message, char *out) {
  uint8_t sum = 0;
  uint8_t message_len = strlen(message);
  for (uint8_t i = 1; message[i] != '*' && i < message_len; i++) {
    sum ^= message[i];
  }
  prv_int_to_hex(sum, out);
}

bool evm_gps_compare_checksum(char* message) {
  char computed[3];
  evm_gps_compute_checksum(message, computed);
  char* received = message + strlen(message) - 2;
  printf("Comparing checksums: received: %s, computed: %s", received, computed);
  return strcmp(computed, received) == 0;
}
