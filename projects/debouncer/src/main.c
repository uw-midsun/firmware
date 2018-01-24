#include <stdbool.h>
#include "gpio.h"
#include "gpio_it.h"
#include "interrupt.h"
#include "log.h"
#include "soft_timer.h"
#include "delay.h"
#include "interrupt_def.h"
#include "stm32f0xx_interrupt.h"
#include "stm32f0xx.h"
#include "wait.h"


typedef struct DebounceInfo {
  GPIOAddress address;
  GPIOState state;
  gpio_it_callback callback;
  void *context;
} DebounceInfo;




static void debouncer_timer_callback(SoftTimerID timer_id, void *context) {
  DebounceInfo *debounce_info = context; 
  GPIOState current_state;
  gpio_get_state(&debounce_info->address, &current_state);
  if (current_state == debounce_info->state) {
      debounce_info->callback(&debounce_info->address, debounce_info->context);  
  }
  EXTI->IMR |= (uint32_t)(1); 
}



//this function should be registered as the interrupt callback on the given address 
void debouncer_it_callback(const GPIOAddress *address,  void *context) {
 // LOG_DEBUG("switched\n");
  DebounceInfo *debounce_info = context; 
  gpio_get_state(address, &debounce_info->state);
  //GPIOState old_state = *(GPIOState*)context;
  EXTI->IMR &= (uint32_t)(~1); //make generic
  //delay_ms(50);
  
  soft_timer_start_millis(50, debouncer_timer_callback, debounce_info, NULL);
 // gpio_get_state(address, &current_state);  
  //if (current_state == old_state) {
  //    LOG_DEBUG("switched\n");   
  //}
  //EXTI->IMR |= (uint32_t)(1); //mask func, add it to stm_interrupt
}


static void prv_handle_statuscode(const Status *status) {
  LOG_DEBUG("Status code: %d from %s (caller %s): %s\n", status->code, status->source, status->caller, status->message);
}

// typedef void(* gpio_it_callback)(const GPIOAddress *address, void *context)


StatusCode debouncer_init_pin(DebounceInfo *debounce_info, const GPIOAddress *address, gpio_it_callback callback, void *context) {
  
  GPIOSettings gpio_settings = {
    .direction = GPIO_DIR_IN,  //
    .resistor = GPIO_RES_NONE,    //
  };

  gpio_init_pin(address, &gpio_settings);

  InterruptSettings interrupt_settings = {
    .type = INTERRUPT_TYPE_INTERRUPT,
    .priority = INTERRUPT_PRIORITY_LOW,
  };

  debounce_info->address = *address;
  debounce_info->callback = callback;
  debounce_info->context = context;

  StatusCode ret = gpio_it_register_interrupt(address, &interrupt_settings, INTERRUPT_EDGE_RISING_FALLING, debouncer_it_callback, debounce_info);
  return ret;
}


// typedef void(* gpio_it_callback)(const GPIOAddress *address, void *context)

void callback(const GPIOAddress *address, void *context){
  LOG_DEBUG("switched\n");
}


int main(void) {
 LOG_DEBUG("Hello World!\n");
 
  // Init GPIO module
  gpio_init();
  interrupt_init();
  soft_timer_init();
  status_register_callback(prv_handle_statuscode);
  // Set up PA0 as an output, default to output 0 (GND)
  GPIOAddress button = {
    .port = GPIO_PORT_A,  //
    .pin = 0,             //
  };

  DebounceInfo db = { 0 };

  debouncer_init_pin(&db, &button, callback, NULL);

  // Add infinite loop so we don't exit
  while (true) {
    wait();
     
  }
 
  return 0;
}

// LOG_DEBUG("swd\n");

    //soft_timer_start_millis(duration_ms, callback, context, timer_id)
    //StatusCode gpio_get_state(const GPIOAddress *address, GPIOState *input_state);
    
    //GPIOState oldstate = state;
    
    //gpio_get_state(&button, &state);
    
    //if (state != oldstate) LOG_DEBUG("%d\n", ++i);
