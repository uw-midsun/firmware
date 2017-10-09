#include <stdio.h>
#include <string.h>
#include "status.h"
#include "gps.h"
#include "gpio.h"
#include "uart_mcu.h"
#include "misc.h"

static const UARTPort port = UART_PORT_2;

// Prototype parser
// Should change the "result field later"
void s_nmea_read(const uint8_t *rx_arr, size_t len, void *context) {
  printf("recieved data with len %zu\n", len);
  for (uint8_t i = 0; i < len; i++) {
    printf("byte %d: %d\n", i, rx_arr[i]);
  }
  
  // Should use context somewhere here to make sure the message is for us
  
  if(len < 1 && rx_arr[0] != '$'){
    printf("Something is fishy, first character of a NMEA sentence should be a $, it is a %c", rx_arr[0]);
  }
  if(len < 3 && (rx_arr[1] != 'G' || rx_arr[2] != 'P')){
    printf("Something is fishy, the second and third characters of a NMEA sentence should be a GP. They are %c%c", rx_arr[1], rx_arr[2]);
  }
  // Array index 3 should be 0
  uint8_t message_id [4] = {0};
  for(uint32_t i = 3; i < 6; i++){
    if(rx_arr[i] == ','){
     break; 
    }
    message_id[i - 3] = rx_arr[i];
  }
  
  //Making sure array index 3 is \n
  message_id[3] = '\n';
  
  //Parsing which type of NMEA message this is
  if(strcmp((char *)message_id, "GGA") == 0){
    result.message_id = GGA;
  }else if(strcmp((char *)message_id, "GLL") == 0){
    result.message_id = GLL;
  }else if(strcmp((char *)message_id, "GSA") == 0){
    result.message_id = GSA;
  }else if(strcmp((char *)message_id, "GSV") == 0){
    result.message_id = GSV;
  }else if(strcmp((char *)message_id, "RMC") == 0){
    result.message_id = RMC;
  }else if(strcmp((char *)message_id, "VTG") == 0){
    result.message_id = VTG;
  }else{
    printf("Unknown message type: %c%c%c", message_id[0], message_id[1], message_id[2]);
    return;
  }
  
  // Do checksum right here
  uint32_t checksum = 0;
  for(uint32_t i = 1; i < len; i++){
    if(rx_arr[i] == '*'){
      break;
    }
    checksum ^= rx_arr[i];
  }
  // Get the checksum of the message
  char* junk = "";
  uint32_t realcheck = 0;
  sscanf((char *) rx_arr, "%s*%d", junk, (int *) &realcheck);
  if(realcheck != checksum){
    printf("Checksums do not match, recieved: %d, calculated: %d", (int) realcheck, (int) checksum);
  }
  // Parse message_id below
  if(result.message_id == GGA){
    // Example message: $GPGGA,053740.000,2503.6319,N,12136.0099,E,1,08,1.1,63.8,M,15.2,M,,0000*64
    char temp_buf[GGA][10] = {{0}};
    uint32_t b = 0;
    
    // Splits individual message components into a 2D array
    for(uint32_t i = 7; i < len; i++){
      if(rx_arr[i] == ','){b++; continue;}
      temp_buf[b][i] = rx_arr[i];
    }
    
    // Parses NMEA message
    sscanf(temp_buf[0], "%2d%2d%2d%3d", (int *) &result.time.hh, (int *) &result.time.mm, (int *) &result.time.ss, (int *) &result.time.sss);
    sscanf(temp_buf[1], "%2d%2d%4d", (int *) &result.latitude.degrees, (int *) &result.latitude.minutes, (int *) &result.latitude.fraction);
    result.north_south = temp_buf[2][0];
    sscanf(temp_buf[3], "%2d%2d%4d", (int *) &result.longtitude.degrees, (int *) &result.longtitude.minutes, (int *) &result.longtitude.fraction);
    result.east_west = temp_buf[4][0];
    sscanf(temp_buf[5], "%d", (int *) &result.position_fix);
    sscanf(temp_buf[6], "%d", (int *) &result.satellites_used);
    sscanf(temp_buf[7], "%f", &result.hdop);
    sscanf(temp_buf[8], "%f", &result.msl_altitude);
    result.units_1 = temp_buf[9][0];
    sscanf(temp_buf[10], "%f", &result.geoid_seperation);
    result.units_2 = temp_buf[11][0];
    sscanf(temp_buf[12], "%d", (int *) &result.adc);
    sscanf(temp_buf[13], "%d", (int *) &result.drs);
  }
}

// FIXME: move this to main.c and pass it in as a arg to evm_gps_init
static UARTSettings s_settings = {
  .baudrate = 9600,
  .rx_handler = s_nmea_read,

  .tx = { .port = GPIO_PORT_A , .pin = 2 },
  .rx = { .port = GPIO_PORT_A , .pin = 3 },
  .alt_fn = GPIO_ALTFN_1,
};

static UARTStorage s_storage = { 0 };

StatusCode evm_gps_init(void) {
  // Makes sure that status codes are handled
  status_ok_or_return(uart_init(port, &s_settings, &s_storage));
  
  uint8_t data[2] = { 42, 24 };
  status_ok_or_return(uart_tx(port, data, SIZEOF_ARRAY(data)));
  return STATUS_CODE_OK;
}
