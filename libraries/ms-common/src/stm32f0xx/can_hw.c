#include "can_hw.h"
#include <string.h>
#include "stm32f0xx.h"
#include "interrupt.h"
#include "log.h"

#define CAN_HW_BASE CAN
#define CAN_HW_PRESCALER 12
#define CAN_HW_NUM_FILTER_BANKS 14

typedef struct CANHwEventHandler {
  CANHwEventHandlerCb callback;
  void *context;
} CANHwEventHandler;

static CANHwEventHandler s_handlers[NUM_CAN_HW_EVENTS];
static uint8_t s_num_filters;

StatusCode can_hw_init(const CANHwSettings *settings) {
  memset(s_handlers, 0, sizeof(s_handlers));
  s_num_filters = 0;

  GPIOSettings gpio_settings = {
    .alt_function = GPIO_ALTFN_4,
    .direction = GPIO_DIR_OUT
  };
  gpio_init_pin(&settings->tx, &gpio_settings);
  gpio_settings.direction = GPIO_DIR_IN;
  gpio_init_pin(&settings->rx, &gpio_settings);

  RCC_APB1PeriphClockCmd(RCC_APB1Periph_CAN, ENABLE);

  RCC_ClocksTypeDef clocks;
  RCC_GetClocksFreq(&clocks);
  // time quanta = (APB1 / Prescaler) / baudrate, -1 for start bit
  uint16_t tq = clocks.PCLK_Frequency / 1000 / CAN_HW_PRESCALER / settings->bus_speed - 1;
  uint16_t bs1 = tq * 7 / 8; // 87.5% sample point

  CAN_DeInit(CAN_HW_BASE);

  CAN_InitTypeDef can_cfg;
  CAN_StructInit(&can_cfg);

  can_cfg.CAN_Mode = settings->loopback ? CAN_Mode_Silent_LoopBack : CAN_Mode_Normal;
  can_cfg.CAN_SJW = CAN_SJW_1tq;
  can_cfg.CAN_BS1 = bs1;
  can_cfg.CAN_BS2 = tq - bs1;
  can_cfg.CAN_Prescaler = CAN_HW_PRESCALER;
  CAN_Init(CAN_HW_BASE, &can_cfg);

  CAN_ITConfig(CAN_HW_BASE, CAN_IT_TME, ENABLE);
  CAN_ITConfig(CAN_HW_BASE, CAN_IT_FMP0, ENABLE);
  CAN_ITConfig(CAN_HW_BASE, CAN_IT_FMP1, ENABLE);
  CAN_ITConfig(CAN_HW_BASE, CAN_IT_BOF, ENABLE);
  // TODO: what is going on and why is my interrupt not firing
  CAN_ITConfig(CAN_HW_BASE, CAN_IT_ERR, ENABLE);
  stm32f0xx_interrupt_nvic_enable(CEC_CAN_IRQn, INTERRUPT_PRIORITY_HIGH);

  can_hw_add_filter(0, 0);
  s_num_filters = 0;

  return STATUS_CODE_OK;
}

StatusCode can_hw_register_callback(CANHwEvent event, CANHwEventHandlerCb callback, void *context) {
  if (event >= NUM_CAN_HW_EVENTS) {
    return status_code(STATUS_CODE_INVALID_ARGS);
  }

  s_handlers[event] = (CANHwEventHandler) {
    .callback = callback,
    .context = context
  };

  return STATUS_CODE_OK;
}

StatusCode can_hw_add_filter(uint16_t mask, uint16_t filter) {
  if (s_num_filters >= CAN_HW_NUM_FILTER_BANKS) {
    return status_msg(STATUS_CODE_RESOURCE_EXHAUSTED, "CAN HW: Ran out of filter banks.");
  }

  // We currently use 32-bit filters. We could use 16-bit filters instead since we're only using
  // standard 11-bit IDs.
  CAN_FilterInitTypeDef filter_cfg = {
    .CAN_FilterNumber = s_num_filters,
    .CAN_FilterMode = CAN_FilterMode_IdMask,
    .CAN_FilterScale = CAN_FilterScale_32bit,
    .CAN_FilterIdHigh = filter << 5,
    .CAN_FilterIdLow = 0x0000,
    .CAN_FilterMaskIdHigh = mask << 5,
    .CAN_FilterMaskIdLow = 0x0000,
    .CAN_FilterFIFOAssignment = (s_num_filters % 2),
    .CAN_FilterActivation = ENABLE
  };
  CAN_FilterInit(&filter_cfg);

  s_num_filters++;
  return STATUS_CODE_OK;
}

CANHwBusStatus can_hw_bus_status(void) {
  if (CAN_GetFlagStatus(CAN_HW_BASE, CAN_FLAG_BOF) == SET) {
    return CAN_HW_BUS_STATUS_OFF;
  } else if (CAN_GetFlagStatus(CAN_HW_BASE, CAN_FLAG_EWG) == SET ||
             CAN_GetFlagStatus(CAN_HW_BASE, CAN_FLAG_EPV) == SET) {
    return CAN_HW_BUS_STATUS_ERROR;
  }

  return CAN_HW_BUS_STATUS_OK;
}

StatusCode can_hw_transmit(uint16_t id, const uint8_t *data, size_t len) {
  CanTxMsg tx_msg = {
    .StdId = id,
    .IDE = CAN_ID_STD,
    .DLC = len
  };

  memcpy(tx_msg.Data, data, len);

  uint8_t tx_mailbox = CAN_Transmit(CAN_HW_BASE, &tx_msg);
  if (tx_mailbox == CAN_TxStatus_NoMailBox) {
      return status_msg(STATUS_CODE_RESOURCE_EXHAUSTED, "CAN HW TX failed");
  }

  return STATUS_CODE_OK;
}

bool can_hw_receive(uint16_t *id, uint64_t *data, size_t *len) {
  // 0: No messages available
  // 1: FIFO0 has received a message
  // 2: FIFO1 has received a message
  // 3: Both have received messages: arbitrarily pick FIFO0
  uint8_t fifo_status = (CAN_MessagePending(CAN_HW_BASE, CAN_FIFO0) != 0) |
                        (CAN_MessagePending(CAN_HW_BASE, CAN_FIFO1) != 0) << 1;
  uint8_t fifo = (fifo_status == 2);

  if (fifo_status == 0) {
    // No messages available
    return false;
  }

  // printf("fifo %d - %d\n", fifo, CAN_MessagePending(CAN_HW_BASE, CAN_FIFO0));
  CanRxMsg rx_msg = { 0 };
  CAN_Receive(CAN_HW_BASE, fifo, &rx_msg);

  *id = rx_msg.StdId;
  *len = rx_msg.DLC;
  memcpy(data, rx_msg.Data, sizeof(*rx_msg.Data) * rx_msg.DLC);

  return true;
}

void CEC_CAN_IRQHandler(void) {
  bool run_cb[NUM_CAN_HW_EVENTS] = {
    [CAN_HW_EVENT_TX_READY] = CAN_GetITStatus(CAN_HW_BASE, CAN_IT_TME) == SET,
    [CAN_HW_EVENT_MSG_RX] = CAN_GetITStatus(CAN_HW_BASE, CAN_IT_FMP0) == SET ||
                            CAN_GetITStatus(CAN_HW_BASE, CAN_IT_FMP1) == SET,
    [CAN_HW_EVENT_BUS_ERROR] = CAN_GetITStatus(CAN_HW_BASE, CAN_IT_BOF) == SET ||
                               CAN_GetITStatus(CAN_HW_BASE, CAN_IT_ERR) == SET
  };

  for (int event = 0; event < NUM_CAN_HW_EVENTS; event++) {
    CANHwEventHandler *handler = &s_handlers[event];
    if (handler->callback != NULL && run_cb[event]) {
      handler->callback(handler->context);
      break;
    }
  }

  CAN_ClearITPendingBit(CAN_HW_BASE, CAN_IT_ERR);
  CAN_ClearITPendingBit(CAN_HW_BASE, CAN_IT_BOF);
  CAN_ClearITPendingBit(CAN_HW_BASE, CAN_IT_TME);
}
