#include "uart.h"

// basic idea: tx is stored in a buffer, interrupt-driven
// rx is buffered, once a newline is hit or the buffer is full, call rx_handler
// run the command once on vagrant startup
// socat -d -d pty,raw,echo=0,link=/home/vagrant/s0 pty,raw,echo=0,link=/home/vagrant/s1 & disown

typedef struct {
  int fd;
  int res;
  char *port;
  struct termios oldtio;
  struct termios newtio;
  UARTStorage *storage;
} UARTPortData;

// /dev/tnt0 and /dev/tnt1 are the connected virtual ports
static UARTPortData s_port[] = {
      [UART_PORT_1] = {.port = "/home/vagrant/s0" }, [UART_PORT_2] = {.port = "/home/vagrant/s1" },
};

static void prv_tx_pop(UARTPort uart);
static void prv_rx_push(UARTPort uart);
static void prv_handle_irq(UARTPort uart);

StatusCode uart_init(UARTPort uart, UARTSettings *settings, UARTStorage *storage) {
  s_port[uart].fd = open(s_port[uart].port, O_RDWR | O_NOCTTY);
  if (s_port[uart].fd < 0) {
    return STATUS_CODE_UNREACHABLE;
  }

  // save current serial port settings
  tcgetattr(s_port[uart].fd, &s_port[uart].oldtio);
  // clear struct for new port settings
  memset(&s_port[uart].newtio, 0, sizeof(s_port[uart].newtio));

  // set baudrate, hardware flow control, 8n1, local connection, enable
  // receiving characters
  s_port[uart].newtio.c_cflag = BAUDRATE | CRTSCTS | CS8 | CLOCAL | CREAD;

  // ignore bytes with parity errors
  // change CR to NL for eol
  s_port[uart].newtio.c_iflag = IGNPAR | ICRNL;

  // raw output
  s_port[uart].newtio.c_oflag = 0;

  // enable cannonical input
  s_port[uart].newtio.c_lflag = ICANON;

  // initialize all contorl characters
  s_port[uart].newtio.c_cc[VINTR] = 0;     // Ctrl-c
  s_port[uart].newtio.c_cc[VQUIT] = 0;     // Ctrl-\ //
  s_port[uart].newtio.c_cc[VERASE] = 0;    // del
  s_port[uart].newtio.c_cc[VKILL] = 0;     // @
  s_port[uart].newtio.c_cc[VEOF] = 4;      // Ctrl-d
  s_port[uart].newtio.c_cc[VTIME] = 0;     // inter-character timer unused
  s_port[uart].newtio.c_cc[VMIN] = 1;      // blocking read until 1 character arrives
  s_port[uart].newtio.c_cc[VSWTC] = 0;     // '\0'
  s_port[uart].newtio.c_cc[VSTART] = 0;    // Ctrl-q
  s_port[uart].newtio.c_cc[VSTOP] = 0;     // Ctrl-s
  s_port[uart].newtio.c_cc[VSUSP] = 0;     // Ctrl-z
  s_port[uart].newtio.c_cc[VEOL] = 0;      // '\0'
  s_port[uart].newtio.c_cc[VREPRINT] = 0;  // Ctrl-r
  s_port[uart].newtio.c_cc[VDISCARD] = 0;  // Ctrl-u
  s_port[uart].newtio.c_cc[VWERASE] = 0;   // Ctrl-w
  s_port[uart].newtio.c_cc[VLNEXT] = 0;    // Ctrl-v
  s_port[uart].newtio.c_cc[VEOL2] = 0;     // '\0'

  // clean modem line and activate settings
  tcflush(s_port[uart].fd, TCIFLUSH);
  tcsetattr(s_port[uart].fd, TCSANOW, &s_port[uart].newtio);

  s_port[uart].storage = storage;
  memset(storage, 0, sizeof(*storage));

  s_port[uart].storage->rx_handler = settings->rx_handler;
  s_port[uart].storage->context = settings->context;
  fifo_init(&s_port[uart].storage->tx_fifo, s_port[uart].storage->tx_buf);
  fifo_init(&s_port[uart].storage->rx_fifo, s_port[uart].storage->rx_buf);

  return STATUS_CODE_OK;
}

StatusCode uart_set_rx_handler(UARTPort uart, UARTRxHandler rx_handler, void *context) {
  s_port[uart].storage->rx_handler = rx_handler;
  s_port[uart].storage->context = context;

  return STATUS_CODE_OK;
}

StatusCode uart_tx(UARTPort uart, uint8_t *tx_data, size_t len) {
  status_ok_or_return(fifo_push_arr(&s_port[uart].storage->tx_fifo, tx_data, len));
  prv_tx_pop(uart);

  return STATUS_CODE_OK;
}

static void prv_tx_pop(UARTPort uart) {
  if (fifo_size(&s_port[uart].storage->tx_fifo) != 0) {
    int res;
    size_t num_bytes = fifo_size(&s_port[uart].storage->tx_fifo);
    uint8_t tx_data[num_bytes];

    fifo_pop_arr(&s_port[uart].storage->tx_fifo, tx_data, num_bytes);

    // send the string
    res = write(s_port[uart].fd, tx_data, num_bytes);

    // ensure a terminating character is subsequent
    if (tx_data[0] != '\n') {
      res = write(s_port[uart].fd, "\n", 1);
    }

    if (res < 0) {
      LOG_DEBUG("Could not send to serial port.");
    }
  }
}

static void prv_rx_push(UARTPort uart) {
  int res;
  uint8_t rx_data;
  res = read(s_port[uart].fd, &rx_data, 1);
  LOG_DEBUG("%c\n", rx_data);
  fifo_push(&s_port[uart].storage->rx_fifo, &rx_data);

  size_t num_bytes = fifo_size(&s_port[uart].storage->rx_fifo);

  if (rx_data == '\n' || num_bytes == UART_MAX_BUFFER_LEN) {
    uint8_t buf[UART_MAX_BUFFER_LEN + 1] = { 0 };
    fifo_pop_arr(&s_port[uart].storage->rx_fifo, buf, num_bytes);

    if (s_port[uart].storage->rx_handler != NULL) {
      s_port[uart].storage->rx_handler(buf, num_bytes, s_port[uart].storage->context);
    }
  }

  if (res < 0) {
    LOG_DEBUG("Could not read from serial port.");
  }
}

static void prv_handle_irq(UARTPort uart) {
  prv_tx_pop(uart);
  prv_rx_push(uart);
}

void USART1_IRQHandler(void) {
  prv_handle_irq(UART_PORT_1);
}

void USART2_IRQHandler(void) {
  prv_handle_irq(UART_PORT_2);
}
