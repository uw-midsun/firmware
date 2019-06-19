#define _DEFAULT_SOURCE
#include "nmea.h"
#include <ctype.h>
#include <inttypes.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "log.h"
#include "misc.h"
#include "nmea_checksum.h"
#include "status.h"
#include "uart.h"  // For UART_MAX_BUFFER_LEN

// Essentially a char, should be defined in inttypes.h but wasn't
#ifndef SCNd8
#define SCNd8 "hhd"
#endif

// Small power function, be careful not to overflow with it.
// Only for positive numbers
// Not useful for edge cases such as 0^0
static uint16_t prv_pow_uint(uint8_t base, int8_t exp) {
  if (exp <= 0) {
    return 1;
  }
  if (base == 0) {
    return 0;
  }
  uint16_t result = base;
  for (uint8_t i = 1; i < exp; i++) {
    result *= base;
  }
  return result;
}

// Turns a string into an integer, but taking place values into account (depending on places arg)
// ex. 0002 => 2, 2000 => 2000, 2 => 2000
static StatusCode prv_string_to_fraction(const char *input, int8_t places, uint16_t *result) {
  if (input == NULL || result == NULL) {
    return status_msg(STATUS_CODE_INVALID_ARGS, "Cannot pass NULL pointers as args");
  }
  if (places == 0) {
    return status_msg(STATUS_CODE_INVALID_ARGS, "Must parse at least one decimal place");
  }
  size_t len = strlen(input);

  if (len == 0) {
    return status_msg(STATUS_CODE_INVALID_ARGS, "Cannot pass empty string as arg");
  }
  for (uint8_t i = 0; i < len && i < places; i++) {
    if (isdigit(input[i])) {
      *result += (input[i] - '0') * prv_pow_uint(10, places - i - 1);
    } else {
      return status_msg(STATUS_CODE_INVALID_ARGS, "Must pass numeric string only");
    }
  }

  return STATUS_CODE_OK;
}

// Just a function to loosely validate if a sentence is valid
StatusCode nmea_valid(const char *to_check) {
  if (to_check == NULL) {
    return status_msg(STATUS_CODE_INVALID_ARGS, "Cannot pass NULL pointer as parameter\n");
  }
  size_t len = strlen(to_check);
  if (len < 3) {
    return status_msg(STATUS_CODE_UNKNOWN,
                      "The length is too short to contain useful information!\n");
  }
  if (to_check[0] != '$') {
    return status_msg(STATUS_CODE_UNKNOWN, "First character of a NMEA sentence should be a $\n");
  }
  if (to_check[1] != 'G' || to_check[2] != 'P') {
    return status_msg(STATUS_CODE_UNKNOWN, "NMEA sentence should begin with GP\n");
  }
  if (!nmea_checksum_validate((char *)to_check, len)) {
    //return status_msg(STATUS_CODE_UNKNOWN, "Invalid checksum for NMEA message\n");
  }
  return STATUS_CODE_OK;
}

// Returns the NMEA sentence type
StatusCode nmea_sentence_type(const char *rx_arr, NmeaMessageId *result) {
  if (result == NULL || rx_arr == NULL) {
    return status_msg(STATUS_CODE_INVALID_ARGS, "Cannot pass NULL pointers to this function\n");
  }

  size_t len = strlen(rx_arr);

  // Array index 3 should be 0, so that it is a null terminated string
  char message_id[4] = { 0 };
  for (uint16_t i = 3; i < len && i < 6; i++) {
    if (rx_arr[i] == ',') {
      break;
    }
    message_id[i - 3] = rx_arr[i];
  }

  // Making sure array index 3 is \0
  message_id[3] = '\0';

  // Parsing which type of NMEA message this is
  if (strcmp(message_id, "GGA") == 0) {
    *result = NMEA_MESSAGE_ID_GGA;
    return STATUS_CODE_OK;
  } else if (strcmp(message_id, "GLL") == 0) {
    *result = NMEA_MESSAGE_ID_GLL;
    return STATUS_CODE_OK;
  } else if (strcmp(message_id, "GSA") == 0) {
    *result = NMEA_MESSAGE_ID_GSA;
    return STATUS_CODE_OK;
  } else if (strcmp(message_id, "GSV") == 0) {
    *result = NMEA_MESSAGE_ID_GSV;
    return STATUS_CODE_OK;
  } else if (strcmp(message_id, "RMC") == 0) {
    *result = NMEA_MESSAGE_ID_RMC;
    return STATUS_CODE_OK;
  } else if (strcmp(message_id, "VTG") == 0) {
    *result = NMEA_MESSAGE_ID_VTG;
    return STATUS_CODE_OK;
  } else {
    //LOG_DEBUG("Unknown message type: %c%c%c", message_id[0], message_id[1], message_id[2]);
    return status_msg(STATUS_CODE_INVALID_ARGS, "Unknown NMEA message type\n");
  }
}

StatusCode nmea_get_gga_sentence(const char *rx_arr, NmeaGgaSentence *result) {
  if (result == NULL || rx_arr == NULL) {
    return status_msg(STATUS_CODE_INVALID_ARGS, "Cannot pass NULL pointers to this function\n");
  }

  size_t len = strlen(rx_arr);

  //status_ok_or_return(nmea_valid(rx_arr));

  // m_id will keep track of which sentence type we are currently operating on
  NmeaMessageId m_id = 0;

  status_ok_or_return(nmea_sentence_type(rx_arr, &m_id));

  result->message_id = m_id;

  // Parses NMEA message

  if (m_id == NMEA_MESSAGE_ID_GGA) {
    // Example message:
    // $GPGGA,053740.000,2503.6319,N,12136.0099,E,1,08,1.1,63.8,M,15.2,M,,0000*64

    char rx_arr_copy[UART_MAX_BUFFER_LEN] = { 0 };
    strncpy(rx_arr_copy, rx_arr, UART_MAX_BUFFER_LEN);

    // This pointer is required because strsep will modify what the pointer
    // points too. This should not be done with the original array
    char *rx_arr_copy_ptr = &rx_arr_copy[0];

    // Get rid of $GPGGA
    char *token = strsep(&rx_arr_copy_ptr, ",");

    // Get time
    token = strsep(&rx_arr_copy_ptr, ",");
    //printf("Time: %s\n", token);
    if (strlen(token) == 10) {
      char hh[2], mm[2], ss[2], sss[3];
      strncpy( hh, token, 2);
      strncpy( mm, token+2, 2);
      strncpy( ss, token+4, 2);
      strncpy( sss, token+7, 3); 
      result->time.hh = atoi(hh);
      result->time.mm = atoi(mm);
      result->time.ss = atoi(ss);
      result->time.sss = atoi(sss);
      // char fraction_string[5] = { 0 };
      // sscanf(token, "%2" SCNd8 "%2" SCNd8 "%2" SCNd8 ".%4s", &result->time.hh, &result->time.mm,
      //        &result->time.ss, fraction_string);
      // uint16_t fraction = 0;
      // prv_string_to_fraction(fraction_string, 3, &fraction);
      // result->time.sss = fraction;
    }

    // Get latitude
    token = strsep(&rx_arr_copy_ptr, ",");
    //printf("Latitude: %s\n", token);
    if (token != NULL) {
      char deg[2], min[2], frac[4];
      strncpy( hh, token, 2);
      strncpy( mm, token+2, 2);
      strncpy( ss, token+4, 2);
      strncpy( sss, token+7, 3); 
      result->time.hh = atoi(hh);
      result->time.mm = atoi(mm);
      result->time.ss = atoi(ss);
      result->time.sss = atoi(sss);
      char fraction_string[5] = { 0 };
      sscanf(token, "%2" SCNd16 "%2" SCNd16 ".%4s", &result->latitude.degrees,
             &result->latitude.minutes, fraction_string);
      uint16_t fraction = 0;
      prv_string_to_fraction(fraction_string, 3, &fraction);
      result->latitude.fraction = fraction;
    }

    // Get North/South indicator
    token = strsep(&rx_arr_copy_ptr, ",");
    //printf("N/S: %s\n", token);
    if (token != NULL) {
      result->north_south = token[0];
    }

    // Get longitude
    token = strsep(&rx_arr_copy_ptr, ",");
    //printf("Longitude: %s\n", token);
    if (token != NULL) {
      char fraction_string[5] = { 0 };
      sscanf(token, "%3" SCNd16 "%2" SCNd16 ".%4s", &result->longitude.degrees,
             &result->longitude.minutes, fraction_string);
      uint16_t fraction = 0;
      prv_string_to_fraction(fraction_string, 3, &fraction);
      result->longitude.fraction = fraction;
    }

    // Get East/West indicator
    token = strsep(&rx_arr_copy_ptr, ",");
    //printf("E/W: %s\n", token);
    if (token != NULL) {
      result->east_west = token[0];
    }

    // Get position fix indicator
    token = strsep(&rx_arr_copy_ptr, ",");
    //printf("GPS Used: %s\n", token);
    if (token != NULL) {
      uint8_t temp_position_fix = 0;

      sscanf(token, "%" SCNd8, &temp_position_fix);

      // Valid position fix defined by 1, 2, or 6 as described in
      // https://www.linxtechnologies.com/wp/wp-content/uploads/rxm-gps-f4.pdf
      // on page 14
      switch (temp_position_fix) {
        case 1:
          result->position_fix = NMEA_POSITION_FIX_GPS_SPS;
          break;
        case 2:
          result->position_fix = NMEA_POSITION_FIX_DIFFERENTIAL_GPS_SPS;
          break;
        case 6:
          result->position_fix = NMEA_POSITION_FIX_DEAD_RECKONING_MODE;
          break;
        default:
          result->position_fix = NMEA_POSITION_FIX_INVALID;
          break;
      }
    }

    // Get satellites used
    token = strsep(&rx_arr_copy_ptr, ",");
    if (token != NULL) {
      sscanf(token, "%" SCNu16, &result->satellites_used);
    }

    // Get horizontal dilution of precision
    token = strsep(&rx_arr_copy_ptr, ",");
    if (token != NULL) {
      char fraction_string[5] = { 0 };
      sscanf(token, "%" SCNd16 ".%4s", &result->hdop_integer, fraction_string);
      uint16_t fraction = 0;
      prv_string_to_fraction(fraction_string, 3, &fraction);
      result->hdop_fraction = fraction;
    }

    // Get mean sea level altitude
    token = strsep(&rx_arr_copy_ptr, ",");
    if (token != NULL) {
      char fraction_string[5] = { 0 };
      sscanf(token, "%" SCNd16 ".%4s", &result->msl_altitude_integer, fraction_string);
      uint16_t fraction = 0;
      prv_string_to_fraction(fraction_string, 3, &fraction);
      result->msl_altitude_fraction = fraction;
    }

    // Get mean sea level altitude units
    token = strsep(&rx_arr_copy_ptr, ",");
    if (token != NULL) {
      result->units_msl_altitude = token[0];
    }

    // Get geoid separation
    token = strsep(&rx_arr_copy_ptr, ",");
    if (token != NULL) {
      char fraction_string[5] = { 0 };
      sscanf(token, "%" SCNd16 ".%4s", &result->geoid_seperation_integer, fraction_string);
      uint16_t fraction = 0;
      prv_string_to_fraction(fraction_string, 3, &fraction);
      result->geoid_seperation_fraction = fraction;
    }

    // Get geoid separation units
    token = strsep(&rx_arr_copy_ptr, ",");
    if (token != NULL) {
      result->units_geoid_seperation = token[0];
    }

    // Get age of differential corrections
    token = strsep(&rx_arr_copy_ptr, ",");
    if (token != NULL) {
      sscanf(token, "%" SCNd16, &result->adc);
    }

    // Get differential reference station
    token = strsep(&rx_arr_copy_ptr, ",");
    if (token != NULL) {
      sscanf(token, "%" SCNd16, &result->drs);
    }

    //printf("RESULT: %d\n", result->north_south);

    return STATUS_CODE_OK;
  }
  return status_msg(STATUS_CODE_INVALID_ARGS, "NMEA: Incorrect message ID received\n");
}

StatusCode nmea_get_vtg_sentence(const char *rx_arr, NmeaVtgSentence *result) {
  if (result == NULL || rx_arr == NULL) {
    return status_msg(STATUS_CODE_INVALID_ARGS, "Cannot pass NULL pointers to function\n");
  }

  size_t len = strlen(rx_arr);

  status_ok_or_return(nmea_valid(rx_arr));

  // m_id will keep track of which sentence type we are currently operating on
  NmeaMessageId m_id = 0;

  status_ok_or_return(nmea_sentence_type(rx_arr, &m_id));

  if (m_id == NMEA_MESSAGE_ID_VTG) {
    // Example message:
    // $GPVTG,79.65,T,,M,2.69,N,5.0,K,A*38

    // Parses NMEA message
    // We only need the direction and speed

    char rx_arr_copy[UART_MAX_BUFFER_LEN] = { 0 };
    strncpy(rx_arr_copy, rx_arr, UART_MAX_BUFFER_LEN);

    // This pointer is required because strsep will modify what the pointer
    // points too. This should not be done with the original array
    char *rx_arr_copy_ptr = &rx_arr_copy[0];
    char *token;

    // First call is to discard $GPVTG
    token = strsep(&rx_arr_copy_ptr, ",");

    // Get course over ground (true)
    token = strsep(&rx_arr_copy_ptr, ",");
    if (token != NULL) {
      char fraction_string[5] = { 0 };
      sscanf(token, "%" SCNd16 ".%4s", &result->measure_heading_degrees_integer, fraction_string);
      uint16_t fraction = 0;
      prv_string_to_fraction(fraction_string, 3, &fraction);
      result->measure_heading_degrees_fraction = fraction;
    }

    // Just discarding data we don't need
    token = strsep(&rx_arr_copy_ptr, ",");  // Reference (true)
    token = strsep(&rx_arr_copy_ptr, ",");  // Course over ground (magnetic)
    token = strsep(&rx_arr_copy_ptr, ",");  // Reference (magnetic)
    token = strsep(&rx_arr_copy_ptr, ",");  // Speed over ground (knots)
    token = strsep(&rx_arr_copy_ptr, ",");  // Units (knots)

    // Get speed over ground (kilometers per hour)
    token = strsep(&rx_arr_copy_ptr, ",");
    if (token != NULL) {
      char fraction_string[5] = { 0 };
      sscanf(token, "%" SCNd16 ".%4s", &result->speed_kmh_integer, fraction_string);
      uint16_t fraction = 0;
      prv_string_to_fraction(fraction_string, 3, &fraction);
      result->speed_kmh_fraction = fraction;
    }

    return STATUS_CODE_OK;
  }

  return status_msg(STATUS_CODE_INVALID_ARGS, "Incorrect message id\n");
}
