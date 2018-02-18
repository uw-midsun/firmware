#include <pthread.h>

#include "gpio.h"
#include "interrupt.h"
#include "gpio_it.h"
#include "log.h"
#include "unity.h"
#include "delay.h"

pthread_t client_thread;
uint32_t client_fd;

// Create socket to set a value externally
// TODO: Create struct to control what action the client takes
static void *prv_client(void *arg) {
  client_fd = socket(AF_INET, SOCK_STREAM, 0);

  connect(client_fd, const struct sockaddr *address,
             socklen_t address_len);

  return NULL;
}

// Create separate thread to create external GPIO signals
void setup_test(void) {
  gpio_init();
  interrupt_init();
  gpio_it_init();
}

// Destroy thread 
void teardown_test(void) {
  // Close client thread
  pthread_join(client_thread, NULL);

  // Close socket
  close(client_fd);
}

void test_set_value(void) {
  GPIOAddress address = { GPIO_PORT_A, 0x0 };
  GPIOSettings gpio_settings = { GPIO_DIR_IN, GPIO_STATE_LOW, GPIO_RES_NONE, GPIO_ALTFN_NONE };
  GPIOState state = GPIO_STATE_LOW;

  // Set GPIO state to low and wait for the client to make a change
  gpio_init_pin(&address, &gpio_settings);
  gpio_set_pin_state(&address, state);

  pthread_create(&client_thread, NULL, prv_client, NULL);

  // Wait for an external response from the client
  delay_ms(1000);

  // Test to see if the value has been correctly written
  gpio_get_value(&address,&state);
  TEST_ASSERT_EQUAL(GPIO_STATE_HIGH, state);

}

void test_interrupt(void) {

}

void test_interrupt_rising_edge(void) {

}

void test_interrupt_falling_edge(void) {

}