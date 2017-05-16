# Specify toolchain
CC := $(GCC_ARM_BASE)arm-none-eabi-gcc
LD := $(GCC_ARM_BASE)arm-none-eabi-gcc
OBJCPY := $(GCC_ARM_BASE)arm-none-eabi-objcopy
OBJDUMP := $(GCC_ARM_BASE)arm-none-eabi-objdump
SIZE := $(GCC_ARM_BASE)arm-none-eabi-size
AR := $(GCC_ARM_BASE)arm-none-eabi-gcc-ar
GDB := $(GCC_ARM_BASE)arm-none-eabi-gdb
OPENOCD := openocd

# Set the library to include if using this platform
PLATFORM_LIB := stm32f0xx
PLATFORM_EXT := .elf

# Architecture dependent variables
ARCH_CLAGS := -mlittle-endian -mcpu=cortex-m0 -march=armv6-m -mthumb

# Linker script location
LDSCRIPT_DIR := $(PLATFORM_DIR)/ldscripts

# Helper scripts location
SCRIPT_DIR := $(PLATFORM_DIR)/scripts

# Build flags for the device
CDEFINES := USE_STDPERIPH_DRIVER STM32F072
CFLAGS := -Wall -Werror -g -Os -std=c99 -Wno-unused-variable -pedantic \
          -ffunction-sections -fdata-sections -fno-builtin -flto \
          --specs=nosys.specs --specs=nano.specs \
          $(ARCH_CLAGS) $(addprefix -D,$(CDEFINES))

# Linker flags
LDFLAGS := $(CLFLAGS) -L$(LDSCRIPT_DIR) -Tstm32f0.ld -fuse-linker-plugin

# Device openocd config file
# Use PROBE=stlink-v2 for discovery boards
PROBE=cmsis-dap
OPENOCD_SCRIPT_DIR := /usr/share/openocd/scripts/
OPENOCD_CFG := -s $(OPENOCD_SCRIPT_DIR) \
               -f interface/$(PROBE).cfg -f target/stm32f0x.cfg \
               -f $(SCRIPT_DIR)/stm32f0-openocd.cfg

# Platform targets
.PHONY: program gdb debug

program: $(BIN_DIR)/$(PROJECT).bin
	@$(OPENOCD) $(OPENOCD_CFG) -c "stm_flash `pwd`/$<" -c shutdown

gdb: $(BIN_DIR)/$(PROJECT)$(PLATFORM_EXT)
	@$(OPENOCD) $(OPENOCD_CFG) > /dev/null 2>&1 &
	@$(GDB) $< -x "$(SCRIPT_DIR)/gdb_flash"
	@pkill openocd

debug: $(BIN_DIR)/$(PROJECT)$(PLATFORM_EXT)
	@$(call session_wrapper,$(GDB) $< -x "$(SCRIPT_DIR)/gdb_flash")

define session_wrapper
tmux new-session -s "ms-fw" -d;
tmux split-window -h -t "ms-fw":0;
tmux send-keys -t "ms-fw":0.1 "echo this should be the serial output" C-m
tmux send-keys -t "ms-fw":0.0 "$(OPENOCD) $(OPENOCD_CFG) > /dev/null 2>&1 &" C-m;
tmux send-keys -t "ms-fw":0.0 '$1; pkill openocd; tmux kill-session -t "ms-fw"' C-m;
tmux select-pane -t 0;
tmux attach -t "ms-fw"
endef

# Defines command to run for unit testing
define test_run
clear && $(GDB) $1 -x "$(SCRIPT_DIR)/gdb_flash" -ex "b LoopForever" -ex "c" -ex "set confirm off" -ex "q"
endef
