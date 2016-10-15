# Defines $(LIB)_SRC, $(LIB)_INC, $(LIB)_DEPS, and $(LIB)_CFLAGS for the library makefile.
# Pre-defined:
# $(LIB)_SRC_ROOT: $(LIB)_DIR/src
# $(LIB)_INC_DIRS: $(LIB)_DIR/inc
# $(LIB)_SRC: $(LIB)_DIR/src/*.c
# $(LIB)_INC: $(LIB)_DIR/inc/*.h

# Find all *.c files in root source and device-specific source folders.
$(LIB)_SRC := $(wildcard $($(LIB)_SRC_ROOT)/*.c) \
              $(wildcard $($(LIB)_SRC_ROOT)/$(DEVICE_FAMILY)/*.c)
$(LIB)_DEPS := $(PLATFORM_LIB)

# Specifies library specific build flags
$(LIB)_CFLAGS :=
