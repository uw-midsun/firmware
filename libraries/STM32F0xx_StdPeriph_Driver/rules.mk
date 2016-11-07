# Defines $(T)_SRC, $(T)_INC, $(T)_DEPS, and $(T)_CFLAGS for the library makefile.
# Pre-defined:
# $(T)_SRC_ROOT: $(T)_DIR/src
# $(T)_INC_DIRS: $(T)_DIR/inc
# $(T)_SRC: $(T)_DIR/src/*.c
# $(T)_INC: $(T)_DIR/inc/*.h

$(T)_DEPS := CMSIS

# Specifies library specific build flags
$(T)_CFLAGS += -ffreestanding -nostdlib
