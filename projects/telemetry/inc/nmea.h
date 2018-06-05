#pragma once
#include <stdbool.h>

// This is a very light weight module to check whether a message is a GGA
// message or VTG message. Just pass the suspect string in and receive a
// true or false result

// Checksums are not computed, this is just done for very basic filtering
// (in order to filter out messages that we do not care about)

// The reason I did not use strstr() is because for messages that we do not
// want, it would iterate over the whole string. We want roughly 2 out of 5
// message types. Therefore using strstr() would have a lot of unnecessary
// overhead

bool nmea_is_gga(const char *to_check);
bool nmea_is_vtg(const char *to_check);
