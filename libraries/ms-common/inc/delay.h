#pragma once
// Delay function library
//
// Blocking delay for a fixed period of time. Still allow interrupts to trigger.
//
// Max allowable time is UINT32_MAX in microseconds.

#include <stdint.h>

// Delay for a period in microseconds.
void delay_us(uint32_t t);

// Delay for a period in milliseconds.
#define delay_ms(time) delay_us((time)*1000)

// Delay for a period in seconds.
#define delay_s(time) delay_us((time)*1000000)
