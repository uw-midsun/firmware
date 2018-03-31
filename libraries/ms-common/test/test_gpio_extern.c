#include <netinet/in.h>
#include <stdbool.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#include <errno.h>

#include "delay.h"
#include "gpio.h"
#include "gpio_it.h"
#include "interrupt.h"
#include "log.h"
#include "status.h"
#include "test_helpers.h"
#include "unity.h"

#define PORT 4000

typedef struct GPIOMessage {
  char cmd[5];
  uint8_t port;
  uint8_t pin;
  uint8_t state;
  uint8_t padding;
} GPIOMessage;

static int client_fd;
static bool callback_ran;
static GPIOMessage message;

static void prv_callback(const GPIOAddress *address, void *context) {
  callback_ran = true;
}

static StatusCode prv_client(void *arg) {
  char *buffer = (char *)arg;
  char reply[sizeof(GPIOMessage)];

  // Send message
  if (write(client_fd, buffer, sizeof(GPIOMessage)) <= 0) {
    LOG_DEBUG("Send failed - %s\n", strerror(errno));
    return status_code(STATUS_CODE_UNREACHABLE);
  }

  // Retry if the read is interrupted
  ssize_t msg_len = 0;

  while (true) {
    msg_len = read(client_fd, reply, sizeof(GPIOMessage));
    if (msg_len >= 0) {
      break;
    }
  }

  return STATUS_CODE_OK;
}

void setup_test(void) {
  // Set up x86 GPIO
  gpio_init();
  interrupt_init();
  gpio_it_init();

  callback_ran = false;

  // Set up client
  client_fd = socket(AF_INET, SOCK_STREAM, 0);

  if (client_fd < 0) {
    LOG_DEBUG("Failed to create socket\n");
    return;
  }

  // Connect to server
  struct sockaddr_in server = {
    .sin_family = AF_INET, .sin_addr.s_addr = INADDR_ANY, .sin_port = htons(PORT), .sin_zero = { 0 }
  };

  if (connect(client_fd, (struct sockaddr *)&server, sizeof(server)) < 0) {
    LOG_DEBUG("Failed to connect to server\n");
    return;
  }

  message = (GPIOMessage){ .cmd = { 'g', 'p', 'i', 'o', ' ' },
                           .port = GPIO_PORT_A,
                           .pin = 0,
                           .state = GPIO_STATE_HIGH,
                           .padding = '\n' };
}

void teardown_test(void) {
  close(client_fd);
}

void test_set_value(void) {
  GPIOAddress address = { GPIO_PORT_A, 0x0 };
  GPIOSettings gpio_settings = { GPIO_DIR_IN, GPIO_STATE_LOW, GPIO_RES_NONE, GPIO_ALTFN_NONE };
  GPIOState state = GPIO_STATE_LOW;

  // Set GPIO state to low and wait for the client to make a change
  gpio_init_pin(&address, &gpio_settings);
  gpio_set_state(&address, state);
  TEST_ASSERT_EQUAL(GPIO_STATE_LOW, state);

  // Change GPIO state with TCP message
  message.state = GPIO_STATE_HIGH;

  TEST_ASSERT_OK(prv_client(&message));

  // Test to see if the value has been correctly written
  gpio_get_state(&address, &state);
  TEST_ASSERT_EQUAL(GPIO_STATE_HIGH, state);
}

void test_interrupt(void) {
  GPIOAddress address = { GPIO_PORT_A, 0x0 };
  GPIOSettings gpio_settings = { GPIO_DIR_IN, GPIO_STATE_LOW, GPIO_RES_NONE, GPIO_ALTFN_NONE };
  InterruptSettings it_settings = { INTERRUPT_TYPE_INTERRUPT, INTERRUPT_PRIORITY_NORMAL };

  gpio_init_pin(&address, &gpio_settings);
  gpio_it_register_interrupt(&address, &it_settings, INTERRUPT_EDGE_RISING_FALLING, prv_callback,
                             NULL);

  message.state = GPIO_STATE_HIGH;

  TEST_ASSERT_OK(prv_client(&message));
  TEST_ASSERT_TRUE(callback_ran);
}

void test_interrupt_rising_edge(void) {
  GPIOAddress address = { GPIO_PORT_A, 0x0 };
  GPIOSettings gpio_settings = { GPIO_DIR_IN, GPIO_STATE_HIGH, GPIO_RES_NONE, GPIO_ALTFN_NONE };
  InterruptSettings it_settings = { INTERRUPT_TYPE_INTERRUPT, INTERRUPT_PRIORITY_NORMAL };
  GPIOState state;

  gpio_init_pin(&address, &gpio_settings);
  gpio_it_register_interrupt(&address, &it_settings, INTERRUPT_EDGE_RISING, prv_callback, NULL);

  // Test that falling edge interrupts do not work
  message.state = GPIO_STATE_LOW;
  TEST_ASSERT_OK(prv_client(&message));
  TEST_ASSERT_FALSE(callback_ran);

  gpio_get_state(&address, &state);
  TEST_ASSERT_EQUAL(GPIO_STATE_LOW, state);

  // Test that rising edge interrupts work
  message.state = GPIO_STATE_HIGH;
  TEST_ASSERT_OK(prv_client(&message));
  TEST_ASSERT_TRUE(callback_ran);

  gpio_get_state(&address, &state);
  TEST_ASSERT_EQUAL(GPIO_STATE_HIGH, state);
}

void test_interrupt_falling_edge(void) {
  GPIOAddress address = { GPIO_PORT_A, 0x0 };
  GPIOSettings gpio_settings = { GPIO_DIR_IN, GPIO_STATE_LOW, GPIO_RES_NONE, GPIO_ALTFN_NONE };
  InterruptSettings it_settings = { INTERRUPT_TYPE_INTERRUPT, INTERRUPT_PRIORITY_NORMAL };
  GPIOState state;

  gpio_init_pin(&address, &gpio_settings);
  gpio_it_register_interrupt(&address, &it_settings, INTERRUPT_EDGE_FALLING, prv_callback, NULL);

  // Test that rising edge interrupts do not work
  message.state = GPIO_STATE_HIGH;
  TEST_ASSERT_OK(prv_client(&message));
  TEST_ASSERT_FALSE(callback_ran);

  gpio_get_state(&address, &state);
  TEST_ASSERT_EQUAL(GPIO_STATE_HIGH, state);

  // Test that falling edge interrupts work
  message.state = GPIO_STATE_LOW;
  TEST_ASSERT_OK(prv_client(&message));
  TEST_ASSERT_TRUE(callback_ran);

  gpio_get_state(&address, &state);
  TEST_ASSERT_EQUAL(GPIO_STATE_LOW, state);
}
