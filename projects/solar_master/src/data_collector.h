#pragma once

#include <stdint.h>
#include <stdio.h>

#include "can_transmit.h"   // For creating and sending CAN message
#include "delay.h"          // For real-time delays

typedef enum {
  SLAVE_0_REQUEST = 0,
  SLAVE_1_REQUEST,
  SLAVE_2_REQUEST,
  SLAVE_3_REQUEST,
  SLAVE_4_REQUEST,
  SLAVE_5_REQUEST,
  SLAVE_0_RESPONSE,
  SLAVE_1_RESPONSE,
  SLAVE_2_RESPONSE,
  SLAVE_3_RESPONSE,
  SLAVE_4_RESPONSE,
  SLAVE_5_RESPONSE,
  NUM_SLAVE_MESSAGES, } SlaveMessages;

// Sends LIN message to the solar slave with the corresponding request id
void slave_send_lin_message(SlaveMessages slave_request_id, uint8_t *arr,
                       size_t arr_size);
					   
// The "template" or design to the callback function
typedef int (*slave_callback)(SlaveMessages message_id, uint8_t *data,
                              size_t data_length, void *context);
							  
void slave_register_callback(SlaveMessages slave_response_id, slave_callback callback,
                             void *context);
							 
// An application of *slave_callback
static int prv_process_message(SlaveMessages slave_response_id, uint8_t *data, size_t data_length,
                               void *context);