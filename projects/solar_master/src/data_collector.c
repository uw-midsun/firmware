// Placeholder code for Solar Master

#include <stdint.h>
#include <stdio.h>

#include "can_transmit.h"    // For creating and sending CAN message
#include "data_collector.h"  // For data collection functions
#include "delay.h"           // For real-time delays

void slave_register_callback(SlaveMessages slave_response_id, slave_callback callback,
                             void *context) {
  // TODO(TYPE-315): callback function to respond to sent back data
}

// Sends LIN message to the solar slave with the corresponding request id
void slave_send_lin_message(SlaveMessages slave_request_id, uint8_t *arr, size_t arr_size) {
  printf("Send request for slave #%d \n", slave_request_id);
}

// An application of *slave_callback
int prv_process_message(SlaveMessages slave_response_id, uint8_t *data, size_t data_length,
                        void *context) {
  // Extract data
  uint8_t slave_board_id = data[0];  // uint8_t instead of uint16_t because there are not many ids
  uint16_t voltage = data[1] << 8 | data[2];
  uint16_t current = data[3] << 8 | data[3];
  uint16_t temperature = data[4] << 8 | data[5];

  // Output data
  printf("Received: Response ID (%d)\n", slave_response_id);
  printf("          Slave ID (%d)\n", slave_board_id);
  printf("          Voltage (%d)\n", voltage);
  printf("          Current (%d)\n", current);
  printf("          Temperature (%d)\n", temperature);

  // Transmits data over CAN
  CAN_TRANSMIT_SOLAR_DATA_FRONT(slave_board_id, voltage, current, temperature);

  return 0;
}
