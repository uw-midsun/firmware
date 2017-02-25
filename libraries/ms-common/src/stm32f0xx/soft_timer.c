#include "soft_timer.h"

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#include "critical_section.h"
#include "interrupt.h"
#include "status.h"
#include "stm32f0xx_interrupt.h"
#include "stm32f0xx_rcc.h"
#include "stm32f0xx_tim.h"

typedef struct Timer {
  uint32_t expiry_time;
  uint32_t rollover_count;
  SoftTimerID previous_timer;
  SoftTimerID next_timer;
  void *context;
  SoftTimerCallback callback;
} Timer;

// Timers on the stm32f0xx is implemented as an ordered doubly linked list. This allows for O(1)
// access to the minimum timer, O(1) deletion of this timer and O(1) access to the next timer. Since
// the queuing of the next timer is the priority this makes it a very good data structure for this
// application. This does come at a cost of worst case O(n) insertion and deletion of arbitrary
// timers but the tradeoff of super fast access to the head node and its immediate successor is
// worthwhile to allow for fast interrupts.

// This is treated like a doubly linked list where this array is the object pool.
static volatile Timer s_soft_timer_array[SOFT_TIMER_MAX_TIMERS];

// Head of the DLL.
static volatile SoftTimerID s_active_timer_id;

// Rollover count. By my calculations we will never roll this over it would take ~584554 yrs.
static volatile uint32_t s_rollover_count;
static volatile uint32_t s_freebitset;

static void prv_start_timer(SoftTimerID timer_id) {
  // Clear any pending interrupts, this will always occur in an interrupt or new timer getting
  // started in a critical section so we need to prevent an accidental trigger if the time expires
  // in the middle of the critical section and we started a new timer which falls before it.
  TIM_ClearITPendingBit(TIM2, TIM_IT_CC1);

  s_active_timer_id = timer_id;
  if (s_soft_timer_array[timer_id].rollover_count == s_rollover_count &&
      s_soft_timer_array[timer_id].expiry_time < TIM_GetCounter(TIM2)) {
    // Expired run immediately.
    s_soft_timer_array[timer_id].callback(timer_id, s_soft_timer_array[timer_id].context);
    s_freebitset |= (1 << timer_id);
    // Start the next timer.
    if (s_soft_timer_array[timer_id].next_timer != SOFT_TIMER_MAX_TIMERS) {
      prv_start_timer(s_soft_timer_array[timer_id].next_timer);
    }
  } else {
    // Set CC1 to trigger an interrupt. Doesn't have to be within uint32_t as it can be multiple
    // rollovers out but this is not enabled at present.
    TIM_SetCompare1(TIM2, s_soft_timer_array[timer_id].expiry_time);
  }
}

// Recursive method to find the correct place to insert the timer_id node in the linked list.
static void prv_insert_timer(SoftTimerID timer_id, SoftTimerID last_node_id) {
  if (s_soft_timer_array[last_node_id].next_timer == SOFT_TIMER_MAX_TIMERS) {
    // Next node is empty. Insert this.
    s_soft_timer_array[last_node_id].next_timer = timer_id;
  } else if (s_soft_timer_array[timer_id].rollover_count <=
                 s_soft_timer_array[s_soft_timer_array[last_node_id].next_timer].rollover_count &&
             s_soft_timer_array[timer_id].expiry_time <
                 s_soft_timer_array[s_soft_timer_array[last_node_id].next_timer].expiry_time) {
    // Next node is after this node and the prior node is before. Insert this, if the timers have
    // the same expiry the last added timer is later in the list.
    s_soft_timer_array[timer_id].next_timer = s_soft_timer_array[last_node_id].next_timer;
    s_soft_timer_array[last_node_id].next_timer = timer_id;
    s_soft_timer_array[s_soft_timer_array[timer_id].next_timer].previous_timer = timer_id;
  } else {
    // Longer than the next node recurse to the next node.
    prv_insert_timer(timer_id, s_soft_timer_array[last_node_id].next_timer);
  }
}

void soft_timer_init(void) {
  // Start the PeiphClock
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);

  // Stop the timer if it was running.
  TIM_Cmd(TIM2, DISABLE);

  // Get the clock speed of the clocks to determine the SYSCLK speed which is what TIM2 uses.
  // Note this is susceptible to prescaling on both the AHB and APB but by default these are not
  // scaled.
  RCC_ClocksTypeDef clock_speeds;
  RCC_GetClocksFreq(&clock_speeds);

  // Configure each clock tick to be 1 microsecond from 0 to UINT32_MAX.
  // Note that while this is valid for now, any prescale changes to AHB and APB will impact the
  // Prescaler here.
  TIM_TimeBaseInitTypeDef init_struct = { .TIM_Prescaler = clock_speeds.SYSCLK_Frequency / 1000000,
                                          .TIM_CounterMode = TIM_CounterMode_Up,
                                          .TIM_Period = UINT32_MAX,
                                          .TIM_ClockDivision = TIM_CKD_DIV1 };
  TIM_TimeBaseInit(TIM2, &init_struct);

  // Reset the counter to 0.
  TIM_SetCounter(TIM2, 0);

  // Clear and disable all the timers and forget the last running timer and rollover count.
  s_rollover_count = 0;
  s_active_timer_id = SOFT_TIMER_MAX_TIMERS;
  s_freebitset |= (1 << SOFT_TIMER_MAX_TIMERS) - 1;
  for (uint32_t i = 0; i < SOFT_TIMER_MAX_TIMERS; i++) {
    s_soft_timer_array[i].next_timer = SOFT_TIMER_MAX_TIMERS;
    s_soft_timer_array[i].previous_timer = SOFT_TIMER_MAX_TIMERS;
  }

  // Enable the interrupts on Capture Compare and updates. TIM2 uses IRQ channel 15.
  stm32f0xx_interrupt_nvic_enable(15, INTERRUPT_PRIORITY_NORMAL);
  TIM_ITConfig(TIM2, TIM_IT_CC1, ENABLE);
  TIM_ClearITPendingBit(TIM2, TIM_IT_CC1);

  // Update on overflows only. Clear any pending overflows.
  TIM_UpdateRequestConfig(TIM2, TIM_UpdateSource_Regular);
  TIM_ClearITPendingBit(TIM2, TIM_IT_Update);

  // Start the timer.
  TIM_Cmd(TIM2, ENABLE);
  // Enable the overflow interrupt after to prevent an accidental overflow trigger.
  TIM_ITConfig(TIM2, TIM_IT_Update, ENABLE);
}

StatusCode soft_timer_start(uint32_t duration_us, SoftTimerCallback callback, void *context,
                            SoftTimerID *timer_id) {
  const uint32_t count = TIM_GetCounter(TIM2);

  // Enable a critical section.
  const bool critical = critical_section_start();
  const uint16_t free_bit = __builtin_ffsl(s_freebitset);
  if (free_bit) {
    // Look for an empty timer.
    const uint16_t i = free_bit - 1;

    // Check for rollover purely to see if we need to increment the rollover count.
    if (duration_us > UINT32_MAX - count) {
      s_soft_timer_array[i].rollover_count = s_rollover_count + 1;
    } else {
      s_soft_timer_array[i].rollover_count = s_rollover_count;
    }

    // Allow the rollover (this is fine for uints).
    s_soft_timer_array[i].expiry_time = count + duration_us;
    s_soft_timer_array[i].next_timer = SOFT_TIMER_MAX_TIMERS;
    s_soft_timer_array[i].previous_timer = SOFT_TIMER_MAX_TIMERS;
    s_soft_timer_array[i].callback = callback;
    s_soft_timer_array[i].context = context;
    s_freebitset &= ~(1 << i);
    *timer_id = i;

    if (s_soft_timer_array[i].expiry_time < s_soft_timer_array[s_active_timer_id].expiry_time &&
        s_soft_timer_array[i].rollover_count <=
            s_soft_timer_array[s_active_timer_id].rollover_count) {
      // New timer is before the current timer so replace it.
      s_soft_timer_array[i].next_timer = s_active_timer_id;
      prv_start_timer(i);
    } else {
      // New timer is not newest just find a free node an insert it.
      prv_insert_timer(i, s_active_timer_id);
    }

    critical_section_end(critical);
    return STATUS_CODE_OK;
  }

  // Out of timers.
  critical_section_end(critical);
  return status_msg(STATUS_CODE_RESOURCE_EXHAUSTED, "Out of software timers.");
}

bool soft_timer_inuse(void) {
  if (s_active_timer_id != SOFT_TIMER_MAX_TIMERS) {
    return true;
  }
  return false;
}

// TIM2 Interrupt handler as defined in stm32f0xx.h
void TIM2_IRQHandler(void) {
  if (TIM_GetITStatus(TIM2, TIM_IT_CC1) != RESET) {
    // Update the timer assuming the active timer expired.
    s_soft_timer_array[s_active_timer_id].callback(s_active_timer_id,
                                                   s_soft_timer_array[s_active_timer_id].context);
    s_freebitset |= (1 << s_active_timer_id);

    // Start the next active timer if it exists as determined by the update.
    if (s_soft_timer_array[s_active_timer_id].next_timer < SOFT_TIMER_MAX_TIMERS) {
      prv_start_timer(s_soft_timer_array[s_active_timer_id].next_timer);
    }

    // Clear the pending bit to stop a repeat trigger if there are no new timers.
    TIM_ClearITPendingBit(TIM2, TIM_IT_CC1);
  }

  if (TIM_GetITStatus(TIM2, TIM_IT_Update) != RESET) {
    // Rolled over.
    s_rollover_count++;
    TIM_ClearITPendingBit(TIM2, TIM_IT_Update);
  }
}

bool soft_timer_cancel(SoftTimerID timer_id) {
  if (timer_id < SOFT_TIMER_MAX_TIMERS) {
    if (timer_id == s_active_timer_id) {
      // Start a critical section and update pretending the active timer doesn't exist so as to
      // replace it with the next timer. Since the head is tracked we don't really have to do any
      // cleanup.
      const bool critical = critical_section_start();
      s_freebitset |= (1 << timer_id);
      prv_start_timer(s_soft_timer_array[timer_id].next_timer);
      critical_section_end(critical);
      return true;
    } else if (s_freebitset & (1 << timer_id)) {
      // Not active pretend it doesn't exist by popping the node out of the doubly linked list.
      s_freebitset |= (1 << timer_id);
      s_soft_timer_array[s_soft_timer_array[timer_id].previous_timer].next_timer =
          s_soft_timer_array[timer_id].next_timer;
      return true;
    }
  }
  return false;
}

uint32_t soft_timer_remaining_time(SoftTimerID timer_id) {
  // Check if the timer is running.
  if (~s_freebitset & (1 << timer_id)) {
    if (s_soft_timer_array[timer_id].rollover_count > s_rollover_count) {
      // If it needs to rollover add the time left to rollover to the expiry time. (Warning not
      // compatible above uint32_t duration sizes).
      return UINT32_MAX - TIM_GetCounter(TIM2) + s_soft_timer_array[timer_id].expiry_time;
    } else {
      // If it doesn't need to rollover just return the time remaining.
      return s_soft_timer_array[timer_id].expiry_time - TIM_GetCounter(TIM2);
    }
  }
  return 0;
}
