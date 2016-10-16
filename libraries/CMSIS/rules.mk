# Defines $(LIB)_SRC, $(LIB)_INC, $(LIB)_DEPS, and $(LIB)_CFLAGS for the library makefile.
# Pre-defined:
# $(LIB)_SRC_ROOT: $(LIB)_DIR/src
# $(LIB)_INC_DIRS: $(LIB)_DIR/inc
# $(LIB)_SRC: $(LIB)_DIR/src/*.c
# $(LIB)_INC: $(LIB)_DIR/inc/*.h

# Redefine the source and include root folders
$(LIB)_SRC_ROOT := $($(LIB)_DIR)/Device/ST/STM32F0xx/Source/Templates
$(LIB)_INC_DIRS := $($(LIB)_DIR)/Device/ST/STM32F0xx/Include \
                   $($(LIB)_DIR)/Include

# Redefine $(LIB)_SRC to reflect the new root
$(LIB)_SRC := $(wildcard $($(LIB)_SRC_ROOT)/*.c)
$(LIB)_INC := $(call find_in,$($(LIB)_INC_DIRS),*.h)
$(LIB)_DEPS :=

# Specifies library specific build flags
$(LIB)_CFLAGS += -ffreestanding -nostdlib
