# Defines $(LIB)_SRC, $(LIB)_DEPS, and $(LIB)_CFLAGS for the library makefile.
# $(LIB)_SRC_ROOT, $(LIB)_INC_ROOT, and $(LIB)_OBJ_ROOT have been pre-defined.

# Find all *.c files in root source and device-specific source folders.
$(LIB)_SRC := $(wildcard $($(LIB)_SRC_ROOT)/*.c) \
              $(wildcard $($(LIB)_SRC_ROOT)/$(DEVICE_FAMILY)/*.c)
$(LIB)_DEPS := stm32f0xx

# Specifies library specific build flags
$(LIB)_CFLAGS := -g -O2 -Wall -Werror -pedantic -Wno-unused-variable \
  $(ARCH) $(DEVICE_LIB) -I$($(LIB)_INC_ROOT) -ffreestanding -nostdlib
