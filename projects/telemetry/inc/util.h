#pragma once
#include <stdbool.h>
#include <stdint.h>

// Computes the checksum for the given message.
// The method can take a NMEA message, it automatically checks the bounds for $ and *
char* compute_checksum(char* message);
bool compare_checksum(char* message);
