# Defines $(T)_SRC, $(T)_INC, $(T)_DEPS, and $(T)_CFLAGS for the build makefile.
# Tests can be excluded by defining $(T)_EXCLUDE_TESTS.
# Pre-defined:
# $(T)_SRC_ROOT: $(T)_DIR/src
# $(T)_INC_DIRS: $(T)_DIR/inc{/$(PLATFORM)}
# $(T)_SRC: $(T)_DIR/src{/$(PLATFORM)}/*.{c,s}

# Specify the libraries you want to include
$(T)_DEPS := ms-common ms-helper

$(T)_INC_DIRS := $($(T)_INC_DIRS) $($(T)_DIR)/inc/fsm
$(T)_SRC_DIRS := $($(T)_SRC_ROOT) $($(T)_SRC_ROOT)/$(PLATFORM) $($(T)_SRC_ROOT)/fsm
$(T)_SRC := $(call find_in,$($(T)_SRC_DIRS),*.c)

ifeq (x86,$(PLATFORM))
$(T)_EXCLUDE_TESTS := control_stalk
endif
