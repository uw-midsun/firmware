# Specify Compiler
TOOLCHAIN_PATH := "/c/Program Files (x86)/GNU Tools ARM Embedded/5.4 2016q2/bin/"
CC := $(TOOLCHAIN_PATH)arm-none-eabi-gcc
OBJCPY := $(TOOLCHAIN_PATH)arm-none-eabi-objcopy
OBJDUMP := $(TOOLCHAIN_PATH)arm-none-eabi-objdump
SIZE := $(TOOLCHAIN_PATH)arm-none-eabi-size
AR := $(TOOLCHAIN_PATH)arm-none-eabi-ar

# Makes life easy if the library moves
STM32F0_DIR := $(LIB_DIR)/stm32f0xx

# Architecture dependent variables
ARCH := -mlittle-endian -mcpu=cortex-m0 -march=armv6-m -mthumb

# Device Library specific variables
INC := -include $(STM32F0_DIR)/stm32f0xx_conf.h \
			-isystem $(STM32F0_DIR)/CMSIS/Include \
			-isystem $(STM32F0_DIR)/CMSIS/Device/ST/STM32F0xx/Include \
			-isystem $(STM32F0_DIR)/STM32F0xx_StdPeriph_Driver/inc \
			-isystem $(STM32F0_DIR)/STM32F0xx_StdPeriph_Driver/src \
			-I $(STM32F0_DIR)

# Linker and startup script locations
STARTUP := $(PLATFORM_DIR)/startup_stm32f0xx.s
LDSCRIPT := $(PLATFORM_DIR)/ldscripts

# Build flags for the device
CFLAGS := -Wall -Werror -g -Os -Wno-unused-variable -pedantic \
				 $(ARCH) -ffunction-sections -fdata-sections -Wl,--gc-sections \
				 -Wl,-Map=$(BIN_DIR)/$(PROJECT_NAME).map

# Linker flags
LINKER := $(INC) -L$(LDSCRIPT) -Tstm32f0.ld

# Device openocd config file
OPENOCD_CFG := $(STM32F0_DIR)/stm32f0-openocd.cfg
