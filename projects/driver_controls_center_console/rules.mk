# Defines $(T)_SRC, $(T)_INC, $(T)_DEPS, and $(T)_CFLAGS for the build makefile.
# Tests can be excluded by defining $(T)_EXCLUDE_TESTS.
# Pre-defined:
# $(T)_SRC_ROOT: $(T)_DIR/src
# $(T)_INC_DIRS: $(T)_DIR/inc{/$(PLATFORM)}
# $(T)_SRC: $(T)_DIR/src{/$(PLATFORM)}/*.{c,s}

# Specify the libraries you want to include
$(T)_DEPS := ms-common codegen-tooling

ifeq (x86,$(PLATFORM))
$(T)_EXCLUDE_TESTS := \
	gpio_expander
endif

$(T)_test_button_led_MOCKS := gpio_expander_init_pin

$(T)_test_button_led_radio_MOCKS := gpio_expander_init_pin
