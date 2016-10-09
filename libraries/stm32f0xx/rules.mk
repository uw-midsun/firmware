# Defines $(LIB)_SRC, $(LIB)_DEPS, and $(LIB)_CFLAGS for the library makefile.
# $(LIB)_SRC_ROOT, $(LIB)_INC_ROOT, and $(LIB)_OBJ_ROOT have been pre-defined.

# Redefine the source and include root folders
$(LIB)_SRC_ROOT := $($(LIB)_DIR)/STM32F0xx_StdPeriph_Driver/src
$(LIB)_INC_ROOT := $($(LIB)_DIR)/STM32F0xx_StdPeriph_Driver/inc

# Find all *.c files in the root source directory
$(LIB)_SRC := $(wildcard $($(LIB)_SRC_ROOT)/*.c)
$(LIB)_DEPS :=

# Specifies library specific build flags
$(LIB)_CFLAGS := -g -O2 -Wall $(ARCH) $(INC) -ffreestanding -nostdlib
