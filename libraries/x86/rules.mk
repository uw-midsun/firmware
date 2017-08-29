# Defines $(T)_SRC, $(T)_INC, $(T)_DEPS, and $(T)_CFLAGS for the build makefile.
# Tests can be excluded by defining $(T)_EXCLUDE_TESTS.
# Pre-defined:
# $(T)_SRC_ROOT: $(T)_DIR/src
# $(T)_INC_DIRS: $(T)_DIR/inc{/$(PLATFORM)}
# $(T)_SRC: $(T)_DIR/src{/$(PLATFORM)}/*.{c,s}

$(T)_DEPS := libcore

# Specifies library specific build flags
$(T)_CFLAGS += -ffreestanding

ifeq (x86,$(PLATFORM))
# Force the include of x86_cmd to allow its constructor to be linked
$(GDB_TARGET): $($(T)_OBJ_ROOT)/x86_cmd.o
endif
