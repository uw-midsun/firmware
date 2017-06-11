// Timers are implemented as a doubly linked list, sorted in order of remaining time until expiry.
// Thus, the head of the list is always defined as the next timer to expire, so we set the timer
// peripheral's compare register to the head's expiry time.
// This gives us O(1) deletion, but we do have O(n) insertion due to the ordered requirement.
// This tradeoff is worth it for faster interrupts.
#include "soft_timer.h"
#include "objpool.h"
#include "stm32f0xx.h"
#include "log.h"

#define SOFT_TIMER_GET_ID(timer) ((timer) - s_storage)
// A expires before B:
// A has a lower rollover count than b
// A and B have the same rollover count and A's expiry time is less than B's
#define SOFT_TIMER_EXPIRES_BEFORE(a, b) \
  (((a)->expiry_rollover_count < (b)->expiry_rollover_count || \
   ((a)->expiry_rollover_count == (b)->expiry_rollover_count && \
    (a)->expiry_us < (b)->expiry_us)))

typedef struct SoftTimer {
  uint32_t expiry_us;
  uint32_t expiry_rollover_count;
  SoftTimerCallback callback;
  void *context;
  struct SoftTimer *next;
  struct SoftTimer *prev;
} SoftTimer;

typedef struct SoftTimerList {
  uint32_t rollover_count;
  SoftTimer *head;
  ObjectPool pool;
} SoftTimerList;

static volatile SoftTimerList s_timers = { 0 };
static volatile SoftTimer s_storage[SOFT_TIMER_MAX_TIMERS] = { 0 };

static void prv_init_periph(void);
static bool prv_insert_timer(SoftTimer *timer);
static void prv_remove_timer(SoftTimer *timer);
static void prv_update_timer(void);

void soft_timer_init(void) {
  memset(&s_timers, 0, sizeof(s_timers));

  objpool_init(&s_timers.pool, s_storage, NULL, NULL);

  prv_init_periph();
}

// Seems to take around 5us to start a timer
StatusCode soft_timer_start(uint32_t duration_us, SoftTimerCallback callback, void *context,
                            SoftTimerID *timer_id) {
  SoftTimer *node = objpool_get_node(&s_timers.pool);
  if (node == NULL) {
    return status_msg(STATUS_CODE_RESOURCE_EXHAUSTED, "Out of software timers.");
  }

  // Set the expected counter value for a expiry - if count + time_us < count, we overflowed
  const uint32_t count = TIM_GetCounter(TIM2);
  node->expiry_us = count + duration_us;
  node->expiry_rollover_count = s_timers.rollover_count + (node->expiry_us < count);
  node->callback = callback;
  node->context = context;

  if (timer_id != NULL) {
    *timer_id = SOFT_TIMER_GET_ID(node);
  }

  bool head = prv_insert_timer(node);
  if (head) {
    prv_update_timer();
  }

  return STATUS_CODE_OK;
}

bool soft_timer_cancel(SoftTimerID timer_id) {
  if (timer_id >= SOFT_TIMER_MAX_TIMERS) {
    return false;
  }

  // technically should be protected
  prv_remove_timer(&s_storage[timer_id]);
  return true;
}

bool soft_timer_inuse(void) {
  return s_timers.head != NULL;
}

uint32_t soft_timer_remaining_time(SoftTimerID timer_id) {
  if (s_storage[timer_id].expiry_us == 0) {
    return 0;
  }

  // technically should be protected?

  if (s_storage[timer_id].expiry_rollover_count > s_timers.rollover_count) {
    return UINT32_MAX - TIM_GetCounter(TIM2) + s_storage[timer_id].expiry_us;
  } else {
    return s_storage[timer_id].expiry_us - TIM_GetCounter(TIM2);
  }
}

static void prv_init_periph(void) {
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);

  TIM_Cmd(TIM2, DISABLE);
  TIM_ITConfig(TIM2, TIM_IT_CC1, DISABLE);
  TIM_ITConfig(TIM2, TIM_IT_Update, DISABLE);

  RCC_ClocksTypeDef clocks;
  RCC_GetClocksFreq(&clocks);

  TIM_TimeBaseInitTypeDef timer_init = {
    .TIM_Prescaler = (clocks.PCLK_Frequency / 1000000) - 1, // 1 Mhz
    .TIM_CounterMode = TIM_CounterMode_Up,
    .TIM_Period = UINT32_MAX,
    .TIM_ClockDivision = TIM_CKD_DIV1
  };
  TIM_TimeBaseInit(TIM2, &timer_init);

  TIM_ClearITPendingBit(TIM2, TIM_IT_CC1);
  TIM_SetCounter(TIM2, 0);
  TIM_ITConfig(TIM2, TIM_IT_CC1, ENABLE);

  // Update on overflows only. Clear any pending overflows.
  TIM_UpdateRequestConfig(TIM2, TIM_UpdateSource_Regular);
  TIM_ClearITPendingBit(TIM2, TIM_IT_Update);

  stm32f0xx_interrupt_nvic_enable(TIM2_IRQn, INTERRUPT_PRIORITY_NORMAL);

  TIM_Cmd(TIM2, ENABLE);

  TIM_ITConfig(TIM2, TIM_IT_Update, ENABLE);
}

// Returns whether it was inserted into the head
static bool prv_insert_timer(SoftTimer *timer) {
  SoftTimer *next = s_timers.head;
  SoftTimer *prev = NULL;

  // iterate through linked list until we hit either the last node
  // or find a node that expires after this timer
  while (next != NULL && SOFT_TIMER_EXPIRES_BEFORE(next, timer)) {
    prev = next;
    next = next->next;
  }

  if (prev == NULL) {
    // Updated the head
    s_timers.head = timer;
  } else {
    // Update previous node
    prev->next = timer;
    timer->prev = prev;
  }

  if (next != NULL) {
    // Update next node
    next->prev = timer;
    timer->next = next;
  }

  return s_timers.head == timer;
}

static void prv_remove_timer(SoftTimer *timer) {
  if (timer == s_timers.head) {
    s_timers.head = timer->next;
  }

  if (timer->prev != NULL) {
    timer->prev->next = timer->next;
  }

  if (timer->next != NULL) {
    timer->next->prev = timer->prev;
  }

  objpool_free_node(&s_timers.pool, timer);
}

static void prv_update_timer(void) {
  SoftTimer *active_timer = s_timers.head;
  TIM_CCxCmd(TIM2, TIM_Channel_1, TIM_CCx_Disable);

  // TODO: figure out why the offset is necessary - most likely the time it takes
  // for comparison and to enable the CCR1
  while (active_timer != NULL &&
         (active_timer->expiry_rollover_count < s_timers.rollover_count ||
          (active_timer->expiry_rollover_count == s_timers.rollover_count &&
           active_timer->expiry_us <= TIM_GetCounter(TIM2) + 2))) {
    active_timer->callback(SOFT_TIMER_GET_ID(active_timer), active_timer->context);

    prv_remove_timer(active_timer);
    active_timer = s_timers.head;
  }

  if (s_timers.head != NULL) {
    TIM_SetCompare1(TIM2, s_timers.head->expiry_us);
    TIM_CCxCmd(TIM2, TIM_Channel_1, TIM_CCx_Enable);
  }
}

void TIM2_IRQHandler(void) {
  if (TIM_GetITStatus(TIM2, TIM_IT_CC1) == SET) {
    SoftTimer *active_timer = s_timers.head;

    if (active_timer != NULL) {
      active_timer->callback(SOFT_TIMER_GET_ID(active_timer), active_timer->context);
      prv_remove_timer(active_timer);
      prv_update_timer();
    }

    TIM_ClearITPendingBit(TIM2, TIM_IT_CC1);
  }

  if (TIM_GetITStatus(TIM2, TIM_IT_Update) == SET) {
    s_timers.rollover_count++;
    TIM_ClearITPendingBit(TIM2, TIM_IT_Update);
  }
}
