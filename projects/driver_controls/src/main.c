
// int16_t percentage_converter(int16_t reading_in_lsb, int16_t zero_value, int16_t hundred_value) {
//   int16_t percentage;
//   if (zero_value > hundred_value) {
//     percentage = ((100 * (reading_in_lsb - hundred_value)) / (hundred_value - zero_value)) + 100;
//   } else {
//     percentage = (100 * (reading_in_lsb - zero_value)) / (hundred_value - zero_value);
//   }

//   if (percentage < 0) {
//     percentage = 0;
//   } else if (percentage > 100) {
//     percentage = 100;
//   }

//   event_raise(INPUT_EVENT_MECHANICAL_BRAKE_PRESSED, percentage);

//   return percentage;
// }

// static void prv_callback_channel(Ads1015Channel channel, void *context) {
//   Ads1015Storage *storage = context;
//   int16_t reading = 0;
//   ads1015_read_raw(storage, channel, &reading);
//   int16_t percentage = percentage_converter(reading, 1253, 418);
//   printf("%d %d\n", reading, percentage);
// }

// int main(void) {
//   Ads1015Storage storage;

//   gpio_init();
//   interrupt_init();
//   gpio_it_init();
//   soft_timer_init();
//   I2CSettings i2c_settings = {
//     .speed = I2C_SPEED_FAST,
//     .scl = { .port = GPIO_PORT_B, .pin = 10 },
//     .sda = { .port = GPIO_PORT_B, .pin = 11 },
//   };
//   i2c_init(I2C_PORT_2, &i2c_settings);
//   GPIOAddress ready_pin = { .port = GPIO_PORT_B, .pin = 2 };

//   ads1015_init(&storage, I2C_PORT_2, ADS1015_ADDRESS_GND, &ready_pin);

//   ads1015_configure_channel(&storage, ADS1015_CHANNEL_0, true, prv_callback_channel, &storage);

//   while (true) {
//   }

//   return 0;
// }

int main(void){
  #include <stdint.h>
#include <stdio.h>

#include "adc.h"
#include "gpio_it.h"
#include "i2c.h"
#include "interrupt.h"

#include "event_arbiter.h"
#include "input_event.h"

#include "direction_fsm.h"
#include "hazard_light_fsm.h"
#include "horn_fsm.h"
#include "mechanical_brake_fsm.h"
#include "pedal_fsm.h"
#include "power_fsm.h"
#include "push_to_talk_fsm.h"
#include "soft_timer.h"
#include "turn_signal_fsm.h"

// Struct of FSMs to be used in the program
typedef struct FSMGroup {
  FSM power;
  FSM pedal;
  FSM direction;
  FSM turn_signal;
  FSM hazard_light;
  FSM mechanical_brake;
  FSM horn;
  FSM push_to_talk;
} FSMGroup;

int main() {
  FSMGroup fsm_group;
  Event e;

  // Initialize the various driver control devices
  gpio_init();
  interrupt_init();
  gpio_it_init();

  soft_timer_init();

  adc_init(ADC_MODE_CONTINUOUS);

  event_queue_init();

  // Initialize FSMs

  for (;;) {
    if (status_ok(event_process(&e))) {
      // Process the event with the input FSMs
    }
  }
}
}
