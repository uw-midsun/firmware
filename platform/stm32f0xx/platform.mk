# Specify toolchain
CC := $(GCC_ARM_BASE)arm-none-eabi-gcc
LD := $(GCC_ARM_BASE)arm-none-eabi-gcc
OBJCPY := $(GCC_ARM_BASE)arm-none-eabi-objcopy
OBJDUMP := $(GCC_ARM_BASE)arm-none-eabi-objdump
SIZE := $(GCC_ARM_BASE)arm-none-eabi-size
AR := $(GCC_ARM_BASE)arm-none-eabi-ar
OPENOCD := $(OPENOCD_BIN)openocd

# Set the library to include if using this platform
PLATFORM_LIB := stm32f0xx

# Architecture dependent variables
ARCH_CLAGS := -mlittle-endian -mcpu=cortex-m0 -march=armv6-m -mthumb

# Linker and startup script locations
# TODO: Move platform startup to somewhere else?
#PLATFORM_STARTUP := $(PLATFORM_DIR)/startup_stm32f0xx.s
LDSCRIPT := $(PLATFORM_DIR)/ldscripts

# Build flags for the device
CDEFINES := USE_STDPERIPH_DRIVER STM32F072
CFLAGS := -Wall -Werror -g -Os -Wno-unused-variable -pedantic \
          -ffunction-sections -fdata-sections \
          -Wl,-Map=$(BIN_DIR)/$(PROJECT).map --specs=nosys.specs \
          $(ARCH_CLAGS) $(addprefix -D,$(CDEFINES))

# Linker flags
LDFLAGS := $(INC) -L$(LDSCRIPT) -Tstm32f0.ld

# Device openocd config file
OPENOCD_CFG := -s $(OPENOCD_BIN)/../scripts/ \
               -f interface/stlink-v2.cfg -f target/stm32f0x.cfg \
               -f $(PLATFORM_DIR)/stm32f0-openocd.cfg
