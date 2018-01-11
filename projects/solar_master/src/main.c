// Placeholder code for Solar Master

#include <stdint.h>
#include <stdio.h>

#include "can_transmit.h"   // For creating and sending CAN message
#include "delay.h"          // For real-time delays
#include "data_collector.h" // For data collection functions

// Number of slaves to iterate through
#define NUM_SLAVES 6

int main(void) {
  // Initialize callback functions for each valid slave response
  for (SlaveMessages slave_num_response = slave_0_response;
  slave_num_response < NUM_SLAVE_MESSAGES; slave_num_response++) {
    slave_register_callback(slave_num_response, prv_process_message, NULL);
  }

  while (1) {
    // Loop through all slaves
    for (SlaveMessages slave_num_request = slave_0_request; slave_num_request < NUM_SLAVES;
     slave_num_request++) {
      slave_send_lin_message(slave_num_request, NULL, 0); // send request message to each slave
    }

    delay_s(10);
  }

  return 0;
}
