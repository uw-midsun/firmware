#include "timer.h"
#include "objpool.h"
#include "stm32f0xx.h"

#define SOFT_TIMER_GET_ID(timer) ((timer) - s_storage)

typedef struct SoftTimer {
  uint32_t expiry_us;
  uint32_t expiry_rollover_count;
  SoftTimerCb callback;
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

StatusCode timer_init(void) {
  memset(&s_timers, 0, sizeof(s_timers));

  objpool_init(&s_timers.pool, s_storage, NULL, NULL);

  prv_init_periph();
}

StatusCode timer_start(uint32_t time_us, SoftTimerCb callback, void *context,
                       SoftTimerID *timer_id) {
  const uint32_t count = TIM_GetCounter(TIM2);

  SoftTimer *node = objpool_get_node(&s_timers.pool);
  if (node == NULL) {
    return status_msg(STATUS_CODE_RESOURCE_EXHAUSTED, "Out of software timers.");
  }

  // Set the expected counter value for a expiry - if count + time_us < count, we overflowed
  node->expiry_us = count + time_us;
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
}

static void prv_init_periph(void) {
  // TODO: use interrupt.h
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);

  TIM_Cmd(TIM2, DISABLE);
  TIM_ITConfig(TIM2, TIM_IT_CC1, DISABLE);

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

  TIM_Cmd(TIM2, ENABLE);

  NVIC_InitTypeDef nvic_init = {
    .NVIC_IRQChannel = TIM2_IRQn,
    .NVIC_IRQChannelCmd = ENABLE
  };
  NVIC_Init(&nvic_init);
}

// Returns whether it was inserted into the head
static bool prv_insert_timer(SoftTimer *timer) {
  SoftTimer *node = s_timers.head;

  if (node == NULL) {
    s_timers.head = timer;
    return true;
  }

  // iterate through linked list until we hit either the last node
  // or find a node that expires after this timer
  // lowest rollover expires first
  // if rollover is the same, then lowest expiry expires first
  while (node->next != NULL &&
         (node->expiry_rollover_count > timer->expiry_rollover_count ||
          (node->expiry_rollover_count == timer->expiry_rollover_count &&
           node->expiry_us > timer->expiry_us))) {
    node = node->next;
  }

  timer->next = node->next;
  timer->prev = node;
  node->next = timer;
  if (timer->next != NULL) {
    timer->next->prev = timer;
  }

  if (s_timers.head->prev != NULL) {
    // Should only have added a single node at most
    s_timers.head = s_timers.head->prev;
    return true;
  }

  return false;
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
