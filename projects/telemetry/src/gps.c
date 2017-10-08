#include <stdio.h>
#include <string.h>
#include "status.h"
#include "gps.h"
#include "gpio.h"
#include "uart_mcu.h"
#include "misc.h"

static const UARTPort port = UART_PORT_2;
static void s_nmea_read(const uint8_t *rx_arr, size_t len, void *context) {
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
  for(int i = 3; i < 6; i++){
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
  }
  // Parse message_id here
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
