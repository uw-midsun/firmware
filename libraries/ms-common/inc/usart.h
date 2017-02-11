#pragma once
// USART Interface
#include <stdint.h>

#include "status.h"
#include "interrupt.h"
#include "gpio.h"

// For setting word length
typedef enum {
	USART_WORD_LENGTH_8b = 0,
	USART_WORD_LENGTH_9b, 
	USART_WORD_LENGTH_7b,
} USARTWordLength;

// For setting number of stop bits
typedef enum {
	USART_STOP_BITS_1 = 0,
	USART_STOP_BITS_2,
	USART_STOP_BITS_1_5,
} USARTStopBits;

// For setting parity mode
typedef enum {
	USART_PARITY_NO = 0,
	USART_PARITY_EVEN,
	USART_PARITY_ODD,
} USARTParity;

// For setting receive or transmit mode
typedef enum {
	USART_MODE_RX = 0,
	USART_MODE_TX,
} USARTMode;

// For setting hardware flow control mode
typedef enum {
	USART_HARDWARE_FLOW_CONTROL_NONE = 0,
	USART_HARDWARE_FLOW_CONTROL_RTS,
	USART_HARDWARE_FLOW_CONTROL_CTS,
	USART_HARDWARE_FLOW_CONTROL_RTS_CTS,
} USARTHardwareFlowControl;

// For enabling/disabling USART clock
typedef enum {
	USART_CLOCK_DISABLE = 0,
	USART_CLOCK_ENABLE,
} USARTClock;

// For setting steady state of clock
typedef enum {
	USART_CPOL_LOW = 0,
	USART_CPOL_HIGH,
} USARTCPolarity;

// For setting when bit capture is made
typedef enum {
	USART_CPHA_1EDGE = 0,
	USART_CPHA_2EDGE,
} USARTCPhase;

// For setting if clock pulse corresponds to last transmitted data bit
typedef enum {
	USART_LastBit_Disable = 0,
	USART_LastBit_Enable,
} USARTLastBit;

// For setting interrupt source
typedef enum {
	USART_IT_TXE = 0;
	USART_IT_RXNE;
	USART_IT_TC;
	USART_IT_IDLE;
	USART_IT_CTS;
	USART_IT_LBD;
	USART_IT_PE;
	USART_IT_ERR;
} USART_IT_Source;

// USART settings 
typedef struct {
	uint32_t baud_rate;
	USARTWordLength word_length;
	USARTStopBits stop_bits;
	USARTParity parity;
	USARTMode mode;
	USARTHardwareFlowControl hardware_flow_control;
	GPIOAddress tx_address;
	GPIOAddress rx_address;
} USARTSettings;

// USART clock settings (for synchronous mode)
typedef struct {
	USARTClock clock;
	USARTCPolarity cpol;
	USARTCPhase cpha;
	USARTLastBit last_bit;
} USARTClockSettings;

typedef void (*usart_it_callback)(uint8_t periph, void *context);

// Registers new callback
StatusCode usart_register_interrupt(uint8_t address, 
	InterruptSettings *settings, InterruptEdge edge, 
	usart_it_callback callback, void *context);

// Triggers interrupt
StatusCode usart_it_trigger_interrupt(uint8_t address);

// Initializes USART with settings
StatusCode usart_init(uint8_t periph, USARTSettings *settings);

// Initializes USART peripheral clock (for synchronous mode)
StatusCode usart_clock_init(uint8_t periph, USARTClockSettings *clock_settings);

// Configures interrupt source
StatusCode usart_it_config(uint8_t periph, USART_IT_Source usart_it, ENABLE);

// Transmits data
StatusCode usart_send_data(USARTSettings *settings, uint16_t data);

// Receives data
StatusCode usart_receive(USARTSettings *settings);
