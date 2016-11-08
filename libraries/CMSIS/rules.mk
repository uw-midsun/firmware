# Defines $(T)_SRC, $(T)_INC, $(T)_DEPS, and $(T)_CFLAGS for the library makefile.
# Pre-defined:
# $(T)_SRC_ROOT: $(T)_DIR/src
# $(T)_INC_DIRS: $(T)_DIR/inc
# $(T)_SRC: $(T)_DIR/src/*.c
# $(T)_INC: $(T)_DIR/inc/*.h

# Redefine the source and include root folders
$(T)_SRC_ROOT := $($(T)_DIR)/Device/ST/STM32F0xx/Source/Templates
$(T)_INC_DIRS := $($(T)_DIR)/Device/ST/STM32F0xx/Include \
                   $($(T)_DIR)/Include

# Redefine $(T)_SRC to reflect the new root
$(T)_SRC := $(wildcard $($(T)_SRC_ROOT)/*.c)
$(T)_INC := $(call find_in,$($(T)_INC_DIRS),*.h)
$(T)_DEPS :=

# Specifies library specific build flags
$(T)_CFLAGS += -ffreestanding -nostdlib
