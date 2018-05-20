#include "nmea.h"
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include "log.h"
#include "nmea_checksum.h"
#include "status.h"

// Splits individual message components into a 2D array
static void prv_split_nmea_into_array(uint8_t *rx_arr, size_t rx_len, size_t temp_buf_w,
                                    size_t temp_buf_h, char temp_buf[][temp_buf_h]) {
  if (rx_arr == NULL || temp_buf == NULL) {
    LOG_CRITICAL("Cannot pass in NULL parameters!");
    return;
  }

  int16_t char_in_message_index = 0;
  uint16_t message_index = 0;

  // i = 7 because the first data character in a NMEA message is in the
  // 7th index
  for (uint16_t i = 7; i < rx_len; char_in_message_index++, i++) {
    if (rx_arr[i] == ',') {
      if (message_index < temp_buf_w && (size_t)char_in_message_index < temp_buf_h &&
          0 <= char_in_message_index) {
        temp_buf[message_index][char_in_message_index] = 0;
      }
      message_index++;
      char_in_message_index = -1;
      continue;
    }
    if (message_index < temp_buf_w && (size_t)char_in_message_index < temp_buf_h &&
        0 <= char_in_message_index) {
      temp_buf[message_index][char_in_message_index] = (char)rx_arr[i];
    }
  }
}

// Just a function to loosely validate if a sentence is valid
StatusCode nmea_valid(const uint8_t *to_check, size_t len) {
  if (to_check == NULL) {
    return status_msg(STATUS_CODE_INVALID_ARGS, "Cannot pass NULL pointer as parameter\n");
  }
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
StatusCode nmea_sentence_type(const uint8_t *rx_arr, size_t len, NMEA_MESSAGE_ID *result) {
  if (result == NULL || rx_arr == NULL) {
    return status_msg(STATUS_CODE_INVALID_ARGS, "Cannot pass NULL pointers to this function\n");
  }

  // Array index 3 should be 0, so that it is a null terminated string
  uint8_t message_id[4] = { 0 };
  for (uint16_t i = 3; i < len && i < 6; i++) {
    if (rx_arr[i] == ',') {
      break;
    }
    message_id[i - 3] = rx_arr[i];
  }

  // Making sure array index 3 is \n
  message_id[3] = '\0';

  // Parsing which type of NMEA message this is
  if (strcmp((char *)message_id, "GGA") == 0) {
    *result = NMEA_GGA;
    return STATUS_CODE_OK;
  } else if (strcmp((char *)message_id, "GLL") == 0) {
    *result = NMEA_GLL;
    return STATUS_CODE_OK;
  } else if (strcmp((char *)message_id, "GSA") == 0) {
    *result = NMEA_GSA;
    return STATUS_CODE_OK;
  } else if (strcmp((char *)message_id, "GSV") == 0) {
    *result = NMEA_GSV;
    return STATUS_CODE_OK;
  } else if (strcmp((char *)message_id, "RMC") == 0) {
    *result = NMEA_RMC;
    return STATUS_CODE_OK;
  } else if (strcmp((char *)message_id, "VTG") == 0) {
    *result = NMEA_VTG;
    return STATUS_CODE_OK;
  } else {
    LOG_DEBUG("Unknown message type: %c%c%c", message_id[0], message_id[1], message_id[2]);
    return status_msg(STATUS_CODE_INVALID_ARGS, "Unknown NMEA message type\n");
  }
}

StatusCode nmea_get_gga_sentence(const uint8_t *rx_arr, size_t len, nmea_gga_sentence *result) {
  if (result == NULL || rx_arr == NULL) {
    return status_msg(STATUS_CODE_INVALID_ARGS, "Cannot pass NULL pointers to this function\n");
  }
  status_ok_or_return(nmea_valid(rx_arr, len));

  // m_id will keep track of which sentence type we are currently operating on
  NMEA_MESSAGE_ID m_id = 0;

  status_ok_or_return(nmea_sentence_type(rx_arr, len, &m_id));

  result->message_id = m_id;

  // Parse message_id below

  char temp_buf[s_nmea_message_num_fields[NMEA_GGA]][10];

  prv_split_nmea_into_array(rx_arr, len, s_nmea_message_num_fields[NMEA_GGA], 10, temp_buf);

  if (m_id == NMEA_GGA) {
    // Example message:
    // $GPGGA,053740.000,2503.6319,N,12136.0099,E,1,08,1.1,63.8,M,15.2,M,,0000*64

    // Parses NMEA message
    sscanf(temp_buf[0], "%2d%2d%2d%3d", (int *)&result->time.hh, (int *)&result->time.mm,
           (int *)&result->time.ss, (int *)&result->time.sss);

    sscanf(temp_buf[1], "%2d%2d.%4d", (int *)&result->latitude.degrees,
           (int *)&result->latitude.minutes, (int *)&result->latitude.fraction);

    result->north_south = (uint8_t)temp_buf[2][0];

    sscanf(temp_buf[3], "%3d%2d.%4d", (int *)&result->longitude.degrees,
           (int *)&result->longitude.minutes, (int *)&result->longitude.fraction);

    result->east_west = (uint8_t)temp_buf[4][0];
    sscanf(temp_buf[5], "%d", (int *)&result->position_fix);
    sscanf(temp_buf[6], "%d", (int *)&result->satellites_used);
    sscanf(temp_buf[7], "%d.%d", (int *)&result->hdop_1, (int *)&result->hdop_2);
    sscanf(temp_buf[8], "%d.%d", (int *)&result->msl_altitude_1, (int *)&result->msl_altitude_2);
    result->units_msl_altitude = (uint8_t)temp_buf[9][0];
    sscanf(temp_buf[10], "%d.%d", (int *)&result->geoid_seperation_1,
           (int *)&result->geoid_seperation_2);
    result->units_geoid_seperation = (uint8_t)temp_buf[11][0];
    sscanf(temp_buf[12], "%d", (int *)&result->adc);
    sscanf(temp_buf[13], "%d", (int *)&result->drs);
    sscanf(temp_buf[13], "%*[^*]*%2x", (int *)&result->checksum);
    return STATUS_CODE_OK;
  }
  return status_msg(STATUS_CODE_INVALID_ARGS, "NMEA: Incorrect message ID received\n");
}

StatusCode nmea_get_vtg_sentence(const uint8_t *rx_arr, size_t len, nmea_vtg_sentence *result) {
  if (result == NULL || rx_arr == NULL) {
    return status_msg(STATUS_CODE_INVALID_ARGS, "Cannot pass NULL pointers to function\n");
  }
  status_ok_or_return(nmea_valid(rx_arr, len));

  // m_id will keep track of which sentence type we are currently operating on
  NMEA_MESSAGE_ID m_id = 0;

  status_ok_or_return(nmea_sentence_type(rx_arr, len, &m_id));

  char temp_buf[s_nmea_message_num_fields[NMEA_VTG]][10];

  prv_split_nmea_into_array(rx_arr, len, s_nmea_message_num_fields[NMEA_VTG], 10, temp_buf);

  if (m_id == NMEA_VTG) {
    // Example message:
    // $GPVTG,79.65,T,,M,2.69,N,5.0,K,A*38

    // Parses NMEA message
    // We only need the direction and speed
    sscanf(temp_buf[0], "%d.%d", (int *)&result->degrees_1, (int *)&result->degrees_2);

    sscanf(temp_buf[6], "%d.%d", (int *)&result->speed_kmh_1, (int *)&result->speed_kmh_2);
    return STATUS_CODE_OK;
  }
  return status_msg(STATUS_CODE_INVALID_ARGS, "Incorrect message id\n");
}
