#include <sys/socket.h>
#include <netinet/in.h>
#include <stdbool.h>
#include <unistd.h>

#include <string.h>

#include "gpio.h"
#include "interrupt.h"
#include "gpio_it.h"
#include "log.h"
#include "unity.h"
#include "delay.h"
#include "status.h"
#include "test_helpers.h"

#define PORT 4000

static int client_fd;

static bool callback_ran;

//static void prv_callback(const GPIOAddress *address, void *context) {
//  callback_ran = true;
//}

/*
static StatusCode prv_client(void *arg, size_t len) {
  char *buffer = (char*)arg;

  // Send message
  if(send(client_fd, buffer, len, 0) < 0) {
      LOG_DEBUG("Send failed");
      return status_code(STATUS_CODE_UNREACHABLE);
  }

  return STATUS_CODE_OK;
}
*/

void setup_test(void) {
  // Set up x86 GPIO
  gpio_init();
  interrupt_init();
  gpio_it_init();

  callback_ran = false;

  // Set up client
  client_fd = socket(AF_INET, SOCK_STREAM, 0);

  if (client_fd == 0) {
    LOG_DEBUG("Failed to create socket\n");
    return;
  }

  // Connect to server
  struct sockaddr_in server = { .sin_family = AF_INET, .sin_addr.s_addr = INADDR_ANY,
                             .sin_port = htons(PORT), .sin_zero = { 0 } };

  if (connect(client_fd, (struct sockaddr *)&server, sizeof(server)) < 0) {
      LOG_DEBUG("Failed to connect to server\n");
      return;
  }
}

void teardown_test(void) {
  // Close socket
  close(client_fd);
}

void test_set_value(void) {/*
  GPIOAddress address = { GPIO_PORT_A, 0x0 };
  GPIOSettings gpio_settings = { GPIO_DIR_IN, GPIO_STATE_LOW, GPIO_RES_NONE, GPIO_ALTFN_NONE };
  GPIOState state = GPIO_STATE_LOW;

  // Set GPIO state to low and wait for the client to make a change
  gpio_init_pin(&address, &gpio_settings);
  gpio_set_pin_state(&address, state);
  TEST_ASSERT_EQUAL(GPIO_STATE_LOW, state);

  // Change GPIO state with TCP message
  char message[] = { 'g', 'p', 'i', 'o', ' ', GPIO_PORT_A, 0, GPIO_STATE_HIGH, 1, '\0'};
  TEST_ASSERT_OK(prv_client(message, sizeof(message)));

  // Test to see if the value has been correctly written
  do {
    gpio_get_value(&address, &state);
  } while (state == GPIO_STATE_LOW);

  TEST_ASSERT_EQUAL(GPIO_STATE_HIGH, state);*/
}

void test_interrupt(void) {/*
  // Configure interrupt settings
  GPIOAddress address = { GPIO_PORT_A, 0x0 };
  GPIOSettings gpio_settings = { GPIO_DIR_IN, GPIO_STATE_LOW, GPIO_RES_NONE, GPIO_ALTFN_NONE };
  InterruptSettings it_settings = { INTERRUPT_TYPE_INTERRUPT, INTERRUPT_PRIORITY_NORMAL };

  gpio_init_pin(&address, &gpio_settings);
  gpio_it_register_interrupt(&address, &it_settings, INTERRUPT_EDGE_RISING_FALLING,
                             prv_callback, NULL);

  TEST_ASSERT_FALSE(callback_ran);

  // Trigger interrupt with TCP
  char message[] = { 'g', 'p', 'i', 'o', ' ', GPIO_PORT_A, 0, GPIO_STATE_HIGH, 1, '\0'};
  prv_client(message, sizeof(message));

  while (!callback_ran) { }

  TEST_ASSERT_TRUE(callback_ran);*/
}

void test_interrupt_rising_edge(void) {/*
  // Configure interrupt settings for rising edge
  GPIOAddress address = { GPIO_PORT_A, 0x0 };
  GPIOSettings gpio_settings = { GPIO_DIR_IN, GPIO_STATE_HIGH, GPIO_RES_NONE, GPIO_ALTFN_NONE };
  InterruptSettings it_settings = { INTERRUPT_TYPE_INTERRUPT, INTERRUPT_PRIORITY_NORMAL };

  gpio_init_pin(&address, &gpio_settings);
  gpio_it_register_interrupt(&address, &it_settings, INTERRUPT_EDGE_RISING, prv_callback, NULL);

  TEST_ASSERT_FALSE(callback_ran);

  // Check that falling edge does not trigger interrupt
  char message[] = { 'g', 'p', 'i', 'o', ' ', GPIO_PORT_A, 0, GPIO_STATE_LOW, 1, '\0'};
  prv_client(message, sizeof(message));
  TEST_ASSERT_FALSE(callback_ran);

  // Check that rising edge does trigger interrupt
  message[7] = GPIO_STATE_HIGH;
  prv_client(message, sizeof(message));
  TEST_ASSERT_TRUE(callback_ran);*/
}

void test_interrupt_falling_edge(void) {/*
  // Configure interrupt settings for falling edge
  GPIOAddress address = { GPIO_PORT_A, 0x0 };
  GPIOSettings gpio_settings = { GPIO_DIR_IN, GPIO_STATE_LOW, GPIO_RES_NONE, GPIO_ALTFN_NONE };
  InterruptSettings it_settings = { INTERRUPT_TYPE_INTERRUPT, INTERRUPT_PRIORITY_NORMAL };
*/
}