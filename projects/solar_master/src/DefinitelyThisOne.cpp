// SolarMaster.c : Send and respond to slave data

#include <stdint.h>
#include <stdio.h>
#include "can_transmit.h"
#include "delay.h"

#define NUM_MODULES 6 //never changes; better than const int

// module enum
typedef enum module {
	module_0_request,
		module_1_request,
		module_2_request,
		module_3_request,
		module_4_request,
		module_5_request,
		module_0_response,
		module_1_response,
		module_2_response,
		module_3_response,
		module_4_response,
		module_5_response,
} module_messages; //create a "datatype" to store each request, respond id


				   // lin_send_to_slave (request_id, arr, arr_size) sends request to get data for
				   // module #,
				   //   request_id; additional inputs: arr, arr_size
				   // lin_send_to_slave: int, int[], int -> void
void lin_send_to_slave(module_messages request_id, uint8_t *arr,
	size_t arr_size) {
	printf("Send request for module #%d \n", request_id);
}


//This is NOT THE CALLBACK FUNCTION; this is the FORMAT for callback functions
//data is a pointer to an array; each element is an 8-bit piece of data
//size_t is used to be unsigned value (always > 0)
//*context holds extra data; no need for it here but good practice
typedef int(*slave_callback)(module_messages message_id, uint8_t *data,
	size_t len, void *context);


//THIS IS THE CALLBACK FUNCTION
//data format:
//first 8 bits (message id); not 16 bits because not large
//second and third 8 bits (voltage)
//fourth and fifth 8 bits (current)
//sixth and seventh 8 bits (temperature)
static int prv_process_message(module_messages id, uint8_t *data, size_t len,
	void *context) {
	printf("Received: Response ID (%d)\n", id);

	//get data
	uint8_t module_id = data[0];
	uint16_t voltage = data[1] << 8 | data[2];
	uint16_t current = data[3] << 8 | data[3];
	uint16_t temperature = data[4] << 8 | data[5];

	printf("          Module ID (%d)\n", module_id);
	printf("          Voltage (%d)\n", voltage);
	printf("          Current (%d)\n", current);
	printf("          Temperature (%d)\n", temperature);

	// send CAN message to FRONT (should not matter)
	CAN_TRANSMIT_SOLAR_DATA_FRONT(module_id, current, voltage, temperature);
}

//THIS IS THE LISTENER
// slave_regist_callback(response_id, callback, *context) receives callback response;
// is this the callback function? .... no. this is the handler?
void slave_register_callback(int response_id, slave_callback callback,
	void *context);

int main(void) {


	for (module_messages module_num_response = module_0_response; module_num_response < module_0_response + NUM_MODULES; module_num_response++) {
		slave_register_callback(module_num_response, prv_process_message, NULL);
	}

	while (1) {

		// increment module_num until it reaches 6
		for (module_messages module_num_request = module_0_request; module_num_request < NUM_MODULES; module_num_request++) {

			lin_send_to_slave(module_num_request, NULL, 0); // send module_num_request
		}

		//delay function for 10 seconds
		delay_s(10);
	}

	return 0;
}
