#include "nmea.h"
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

// This is to get the number of fields in a sentence for array allocation
static const size_t s_nmea_message_num_fields[] = { 0, 16, 9, 11, 13, 14, 14, 10 };

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
    return status_msg(STATUS_CODE_UNKNOWN, "Invalid checksum for NMEA message\n");
  }
  return STATUS_CODE_OK;
}

// Returns the NMEA sentence type
StatusCode nmea_sentence_type(const char *rx_arr, NmeaMessageID *result) {
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

  // Making sure array index 3 is \n
  message_id[3] = '\0';

  // Parsing which type of NMEA message this is
  if (strcmp(message_id, "GGA") == 0) {
    *result = NMEA_GGA;
    return STATUS_CODE_OK;
  } else if (strcmp(message_id, "GLL") == 0) {
    *result = NMEA_GLL;
    return STATUS_CODE_OK;
  } else if (strcmp(message_id, "GSA") == 0) {
    *result = NMEA_GSA;
    return STATUS_CODE_OK;
  } else if (strcmp(message_id, "GSV") == 0) {
    *result = NMEA_GSV;
    return STATUS_CODE_OK;
  } else if (strcmp(message_id, "RMC") == 0) {
    *result = NMEA_RMC;
    return STATUS_CODE_OK;
  } else if (strcmp(message_id, "VTG") == 0) {
    *result = NMEA_VTG;
    return STATUS_CODE_OK;
  } else {
    LOG_DEBUG("Unknown message type: %c%c%c", message_id[0], message_id[1], message_id[2]);
    return status_msg(STATUS_CODE_INVALID_ARGS, "Unknown NMEA message type\n");
  }
}

StatusCode nmea_get_gga_sentence(const char *rx_arr, nmea_gga_sentence *result) {
  if (result == NULL || rx_arr == NULL) {
    return status_msg(STATUS_CODE_INVALID_ARGS, "Cannot pass NULL pointers to this function\n");
  }

  size_t len = strlen(rx_arr);

  status_ok_or_return(nmea_valid(rx_arr));

  // m_id will keep track of which sentence type we are currently operating on
  NmeaMessageID m_id = 0;

  status_ok_or_return(nmea_sentence_type(rx_arr, &m_id));

  result->message_id = m_id;

  // Parses NMEA message

  if (m_id == NMEA_GGA) {
    // Example message:
    // $GPGGA,053740.000,2503.6319,N,12136.0099,E,1,08,1.1,63.8,M,15.2,M,,0000*64

    char *rx_arr_copy = strdup(rx_arr);

    // strsep modifies original pointer, so we have to keep a copy
    char *rx_arr_copy_pointer_backup = rx_arr_copy;

    // Get rid of $GPGGA
    char *token = strsep(&rx_arr_copy, ",");

    token = strsep(&rx_arr_copy, ",");
    if (token != NULL) {
      sscanf(token, "%2d%2d%2d.%3d", (int *)&result->time.hh, (int *)&result->time.mm,
             (int *)&result->time.ss, (int *)&result->time.sss);
    }

    token = strsep(&rx_arr_copy, ",");
    if (token != NULL) {
      sscanf(token, "%2d%2d.%4d", (int *)&result->latitude.degrees,
             (int *)&result->latitude.minutes, (int *)&result->latitude.fraction);
    }

    token = strsep(&rx_arr_copy, ",");
    if (token != NULL) {
      result->north_south = token[0];
    }

    token = strsep(&rx_arr_copy, ",");
    if (token != NULL) {
      sscanf(token, "%3d%2d.%4d", (int *)&result->longitude.degrees,
             (int *)&result->longitude.minutes, (int *)&result->longitude.fraction);
    }

    token = strsep(&rx_arr_copy, ",");
    if (token != NULL) {
      result->east_west = token[0];
    }

    token = strsep(&rx_arr_copy, ",");
    if (token != NULL) {
      uint8_t temp_position_fix = 0;

      sscanf(token, "%d", (int *)&temp_position_fix);

      // Valid position fix defined by 1, 2, or 6 as described in
      // https://www.linxtechnologies.com/wp/wp-content/uploads/rxm-gps-f4.pdf
      // on page 14
      switch (temp_position_fix) {
        case 1:
          result->position_fix = NMEA_PF_GPS_SPS;
          break;
        case 2:
          result->position_fix = NMEA_PF_DIFFERENTIAL_GPS_SPS;
          break;
        case 6:
          result->position_fix = NMEA_PF_DEAD_RECKONING_MODE;
          break;
        default:
          result->position_fix = NMEA_PF_INVALID;
          break;
      }
    }

    token = strsep(&rx_arr_copy, ",");

    if (token != NULL) {
      sscanf(token, "%d", (int *)&result->satellites_used);
    }

    token = strsep(&rx_arr_copy, ",");
    if (token != NULL) {
      sscanf(token, "%d.%d", (int *)&result->hdop_1, (int *)&result->hdop_2);
    }

    token = strsep(&rx_arr_copy, ",");
    if (token != NULL) {
      sscanf(token, "%d.%d", (int *)&result->msl_altitude_1, (int *)&result->msl_altitude_2);
    }

    token = strsep(&rx_arr_copy, ",");
    if (token != NULL) {
      result->units_msl_altitude = (uint8_t)token[0];
    }

    token = strsep(&rx_arr_copy, ",");
    if (token != NULL) {
      sscanf(token, "%d.%d", (int *)&result->geoid_seperation_1,
             (int *)&result->geoid_seperation_2);
    }

    token = strsep(&rx_arr_copy, ",");
    if (token != NULL) {
      result->units_geoid_seperation = (uint8_t)token[0];
    }

    token = strsep(&rx_arr_copy, ",");
    if (token != NULL) {
      sscanf(token, "%d", (int *)&result->adc);
    }

    token = strsep(&rx_arr_copy, ",");
    if (token != NULL) {
      sscanf(token, "%d", (int *)&result->drs);
    }

    free(rx_arr_copy_pointer_backup);
    return STATUS_CODE_OK;
  }
  return status_msg(STATUS_CODE_INVALID_ARGS, "NMEA: Incorrect message ID received\n");
}

StatusCode nmea_get_vtg_sentence(const char *rx_arr, nmea_vtg_sentence *result) {
  if (result == NULL || rx_arr == NULL) {
    return status_msg(STATUS_CODE_INVALID_ARGS, "Cannot pass NULL pointers to function\n");
  }

  size_t len = strlen(rx_arr);

  status_ok_or_return(nmea_valid(rx_arr));

  // m_id will keep track of which sentence type we are currently operating on
  NmeaMessageID m_id = 0;

  status_ok_or_return(nmea_sentence_type(rx_arr, &m_id));

  if (m_id == NMEA_VTG) {
    // Example message:
    // $GPVTG,79.65,T,,M,2.69,N,5.0,K,A*38

    // Parses NMEA message
    // We only need the direction and speed

    char *rx_arr_copy = strdup(rx_arr);

    // strsep modifies original pointer, so we have to keep a copy
    char *rx_arr_copy_pointer_backup = rx_arr_copy;
    char *token;

    // First call is to discard $GPVTG
    token = strsep(&rx_arr_copy, ",");
    token = strsep(&rx_arr_copy, ",");
    if (token != NULL) {
      sscanf(token, "%d.%d", (int *)&result->measure_heading_degrees_1,
             (int *)&result->measure_heading_degrees_2);
    }

    // Just discarding data we don't need
    token = strsep(&rx_arr_copy, ",");
    token = strsep(&rx_arr_copy, ",");
    token = strsep(&rx_arr_copy, ",");
    token = strsep(&rx_arr_copy, ",");
    token = strsep(&rx_arr_copy, ",");
    token = strsep(&rx_arr_copy, ",");
    if (token != NULL) {
      sscanf(token, "%d.%d", (int *)&result->speed_kmh_1, (int *)&result->speed_kmh_2);
    }
    free(rx_arr_copy_pointer_backup);
    return STATUS_CODE_OK;
  }

  return status_msg(STATUS_CODE_INVALID_ARGS, "Incorrect message id\n");
}
