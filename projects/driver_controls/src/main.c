#include <stdint.h>
#include <stdio.h>

#include "adc.h"
#include "gpio_it.h"
#include "i2c.h"
#include "interrupt.h"

#include "analog_io.h"
#include "digital_io.h"
#include "event_arbiter.h"
#include "input_event.h"

#include "ads1015.h"
#include "ads1015_def.h"
#include "can_output.h"
#include "critical_section.h"
#include "direction_fsm.h"
#include "hazard_light_fsm.h"
#include "horn_fsm.h"
#include "log.h"
#include "mechanical_brake_fsm.h"
#include "pedal_fsm.h"
#include "power_fsm.h"
#include "push_to_talk_fsm.h"
#include "soft_timer.h"
#include "turn_signal_fsm.h"
#include "delay.h"

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
static bool callback_called[4] = { false, false, false, false };
static void prv_callback_main(Ads1015Channel channel, void *context) {
  //LOG_DEBUG("yuppppppppp\n");
  bool *callback_called = context;
 // (*callback_called) = true;
 // LOG_DEBUG("channel %d\n", channel);

}

static void prv_timer_callback(SoftTimerID id, void *context) {
  LOG_DEBUG("yuppppppppp\n");

  prv_callback_main(2, &callback_called[0]);
  soft_timer_start(10, prv_timer_callback, NULL, NULL);
}


int main() {
  FSMGroup fsm_group;
  Event e;

  // Initialize the various driver control devices
  gpio_init();
  interrupt_init();
  gpio_it_init();
  Ads1015Storage storage;

  soft_timer_init();
  soft_timer_start(10, prv_timer_callback, NULL, NULL);

  

  // ads1015_init(&storage, 0, 0, NULL);
  //ads1015_configure_channel(&storage, ADS1015_CHANNEL_0, true, prv_callback_main,
   //                         &callback_called[0]);
  /*ads1015_configure_channel(&storage, ADS1015_CHANNEL_1, false, prv_callback_main,
                            &callback_called[1]);
  ads1015_configure_channel(&storage, ADS1015_CHANNEL_2, true, prv_callback_main,
                            &callback_called[2]);
  ads1015_configure_channel(&storage, ADS1015_CHANNEL_3, false, prv_callback_main,
                            &callback_called[3]);
*/
  while (1) {
    //LOG_DEBUG("hey\n");
    // callback_called[0] = false;
  }

  adc_init(ADC_MODE_CONTINUOUS);

  digital_io_init();
  analog_io_init();

  event_queue_init();

  // Initialize FSMs
  event_arbiter_init(can_output_transmit);

  power_fsm_init(&fsm_group.power);
  pedal_fsm_init(&fsm_group.pedal);
  direction_fsm_init(&fsm_group.direction);
  turn_signal_fsm_init(&fsm_group.turn_signal);
  hazard_light_fsm_init(&fsm_group.hazard_light);
  mechanical_brake_fsm_init(&fsm_group.mechanical_brake);
  horn_fsm_init(&fsm_group.horn);
  push_to_talk_fsm_init(&fsm_group.push_to_talk);

  int16_t adc_data[NUM_ADS1015_CHANNELS];

  for (;;) {
    if (status_ok(event_process(&e))) {
      // Process the event with the input FSMs
      event_arbiter_process_event(&e);
    }
  }
}
