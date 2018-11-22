# Defines $(T)_SRC, $(T)_INC, $(T)_DEPS, and $(T)_CFLAGS for the build makefile.
# Tests can be excluded by defining $(T)_EXCLUDE_TESTS.
# Pre-defined:
# $(T)_SRC_ROOT: $(T)_DIR/src
# $(T)_INC_DIRS: $(T)_DIR/inc{/$(PLATFORM)}
# $(T)_SRC: $(T)_DIR/src{/$(PLATFORM)}/*.{c,s}

# Specify the libraries you want to include
$(T)_DEPS := ms-common ms-helper

$(T)_test_display_brightness_MOCKS := adc_read_raw adc_register_callback adc_get_channel adc_set_channel pwm_init pwm_set_dc
$(T)_test_display_calibration_MOCKS := adc_read_raw adc_register_callback adc_get_channel adc_set_channel pwm_init pwm_set_dc soft_timer_start
