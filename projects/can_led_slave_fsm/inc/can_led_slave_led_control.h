#pragma once
#include "gpio.h"        // General Purpose I/O control.
#include "can.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include "can_unpack.h"

uint16_t LED_GREEN = 0x1; 

struct debug_leds {
    GpioAddress debug_led[4]; 
  }; 
  
StatusCode led_cb(const CanMessage *msg, void *context, CanAckStatus *ack_reply); 