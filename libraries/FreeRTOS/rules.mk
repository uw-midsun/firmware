# Defines $(T)_SRC, $(T)_INC, $(T)_DEPS, and $(T)_CFLAGS for the build makefile.
# Tests can be excluded by defining $(T)_EXCLUDE_TESTS.
# Pre-defined:
# $(T)_SRC_ROOT: $(T)_DIR/src
# $(T)_INC_DIRS: $(T)_DIR/inc{/$(PLATFORM)}
# $(T)_SRC: $(T)_DIR/src{/$(PLATFORM)}/*.{c,s}

$(T)_SRC_ROOT := $($(T)_DIR)

$(T)_INC_DIRS := $($(T)_DIR) \
                  $($(T)_DIR)/include

$(T)_SRC := $(wildcard $($(T)_SRC_ROOT)/*.c) \
            $(wildcard $($(T)_SRC_ROOT)/*.s)

# TODO: Integrate the Linux simulator
ifeq (stm32f0xx,$(PLATFORM))
    $(T)_INC_DIRS += $($(T)_DIR)/portable/GCC/ARM_CM0
    $(T)_SRC += $(wildcard $($(T)_SRC_ROOT)/portable/GCC/ARM_CM0/*.c)
endif

$(T)_DEPS := CMSIS stm32f0xx

# Specifies library specific build flags
$(T)_CFLAGS += -ffreestanding -nostdlib
