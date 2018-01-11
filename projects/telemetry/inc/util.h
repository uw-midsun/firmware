#pragma once
#include <stdbool.h>
#include <stdint.h>

// Computes the checksum for the given message.
// The method can take a NMEA message, it automatically checks the bounds for $ and *
char* evm_gps_compute_checksum(char* message);
bool evm_gps_compare_checksum(char* message);
