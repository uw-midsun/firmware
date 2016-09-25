###################################################################################################
# AUTHOR: Calder Kitagawa
# PURPOSE: CORTEX M0 package template using stm32f0xx libraries
# DATE: SEPT 24 2016
# MODIFIED: 
#
# USAGE:
# 	make [all] - makes the stm32f0xx libraries if not cached and compiler files in src/inc dirs 
#	make remake - rebuilds .elf using src and inc files explicitly by removing and rebuilding
#	make clean - removes the .elf and associated linker and object files
#	make reallyclean - in addition to running make clean also removed the cached libraries
#	make program - builds an OpenOCD binary
#
###################################################################################################

# CONFIG

# put the main files you want to compile here separated by a space
SRCS = main.c

# the name you want the generated .elf file to go by should go here
PROJECT_NAME=main

# location of the libraries
STD_PERIPH_LIB=libraries/stm32f0xx

# linker scripts location if you move the linkers
# DO NOT TOUCH UNLESS YOU ABSOLUTELY KNOW WHAT YOU ARE DOING
LDSCRIPT_INC=linker/stm32f0xx/ldscripts

###################################################################################################

# OPENOCD SSUPPORT

# location of OpenOCD board .cfg files (only triggered if you use 'make program' explicitly)
OPENOCD_BOARD_DIR=/usr/share/openocd/scripts/board

# configuration (cfg) file containing directives for OpenOCD
OPENOCD_PROC_FILE=extra/stm32f0-opencd.cfg

# the rest of this is taken care of please don't touch anything else
###################################################################################################

# GENERAL COMPILER SETUP

# Use gcc-arm for everything
CC=arm-none-eabi-gcc
OBJCPY=arm-none-eabi-objcopy
OBJDUMP=arm-none-eabi-objdump
SIZE=arm-none-eabi-size
AR=arm-none-eabi-ar

# TODO determine if this is the correct architecture
ARCH_FLAGS = -mlittle-endian -mcpu=cortex-m0 -march=armv6-m -mthumb

CFLAGS = -Wall -Werror -g -Os -Wno-unused-variable -pedantic
CFLAGS += $(ARCH_FLAGS)
CFLAGS += -ffunction-sections -fdata-sections
CFLAGS += -Wl,--gc-sections -Wl,-Map=$(PROJECT_NAME).map

###################################################################################################

# TEMPLATE SETUP

vpath %.c src
vpath %.a $(STD_PERIPH_LIB)

ROOT=$(shell pwd)

# TODO Investigating making this more dynamic
CFLAGS += -I inc -isystem $(STD_PERIPH_LIB) \
		-isystem $(STD_PERIPH_LIB)/CMSIS/Device/ST/STM32F0xx/Include
CFLAGS += -isystem $(STD_PERIPH_LIB)/CMSIS/Include \
		-isystem $(STD_PERIPH_LIB)/STM32F0xx_StdPeriph_Driver/inc
CFLAGS += -include $(STD_PERIPH_LIB)/stm32f0xx_conf.h

# Include the startup file in the make
SRCS += linker/stm32f0xx/startup_stm32f0xx.s

OBJS = $(SRCS:.c=.o)

###################################################################################################

# STM32F0xx LIB SETUP

# Use separate flags to build the STD_PERIPH_LIB
# TODO investigate making this more dynamic
LIB_CFLAGS = -g -O2 -Wall
LIB_CFLAGS += $(ARCH_FLAGS)
LIB_CFLAGS += -ffreestanding -nostdlib
LIB_CFLAGS += -include$(STD_PERIPH_LIB)/stm32f0xx_conf.h \
		-I$(STD_PERIPH_LIB)/CMSIS/Include -I$(STD_PERIPH_LIB)/CMSIS/Device/ST/STM32F0xx/Include \
		-I$(STD_PERIPH_LIB)/STM32F0xx_StdPeriph_Driver/inc \
		-I$(STD_PERIPH_LIB)/STM32F0xx_StdPeriph_Driver/src

# STD_PERIPH_LIB source files
LIB_SRCS = stm32f0xx_adc.c stm32f0xx_cec.c stm32f0xx_comp.c stm32f0xx_crc.c \
	stm32f0xx_dac.c stm32f0xx_dbgmcu.c stm32f0xx_dma.c stm32f0xx_exti.c \
	stm32f0xx_flash.c stm32f0xx_gpio.c stm32f0xx_i2c.c stm32f0xx_iwdg.c \
	stm32f0xx_misc.c stm32f0xx_pwr.c stm32f0xx_rcc.c stm32f0xx_rtc.c \
	stm32f0xx_spi.c stm32f0xx_syscfg.c stm32f0xx_tim.c \
	stm32f0xx_usart.c stm32f0xx_wwdg.c

# Store object files in a obj directory
LIB_OBJS = $(addprefix $(STD_PERIPH_LIB)/obj/, $(LIB_SRCS:.c=.o))

###################################################################################################

# MAKE RULES

.PHONY: lint proj

all: lint $(STD_PERIPH_LIB)/libstm32f0.a proj

lint:
	-find inc -name "*.c" -o -name "*.h" | xargs -r cpplint --extensions=c,h
	-find src -name "*.c" -o -name "*.h" | xargs -r cpplint --extensions=c,h

# compiles library objects
$(STD_PERIPH_LIB)/obj/%.o : $(STD_PERIPH_LIB)/STM32F0xx_StdPeriph_Driver/src/%.c
	$(CC) -w -c -o $@ $< $(LIB_CFLAGS) 

# builds the library
$(STD_PERIPH_LIB)/libstm32f0.a: $(LIB_OBJS)
	$(AR) -r $@ $(LIB_OBJS)

# call to build the project
proj:   $(PROJECT_NAME).elf

# This is what actually builds the project
# The necessity of an OBJDUMP and SIZE file for debug is questionable as it is only
# a feature for power-users who would know how to enable it anyway
$(PROJECT_NAME).elf: $(SRCS)
	$(CC) $(CFLAGS) $^ -o $@ -L$(STD_PERIPH_LIB) -lstm32f0 -L$(LDSCRIPT_INC) -Tstm32f0.ld
	$(OBJCPY) -O binary $(PROJECT_NAME).elf $(PROJECT_NAME).bin
	$(OBJDUMP) -St $(PROJECT_NAME).elf >$(PROJECT_NAME).lst
	$(SIZE) $(PROJECT_NAME).elf

# OPTIONAL: add this line to the above rule to compile a hex file as well
# $(OBJCOPY) -O ihex $(PROJ_NAME).elf $(PROJ_NAME).hex

###################################################################################################

# OPENOCD SUPPORT

program: $(PROJECT_NAME).bin
	openocd -f $(OPENOCD_BOARD_DIR)/stm32f0discovery.cfg -f $(OPENOCD_PROC_FILE) \
	-c "stm_flash `pwd`/$(PROJ_NAME).bin" -c shutdown

###################################################################################################

# EXTRA

# clean and remake rules, use reallyclean to remake the STD_PERIPH_LIB or before a push 

clean:
	find ./ -name '*~' | xargs rm -f
	rm -f *.o
	rm -f $(PROJECT_NAME).elf
	rm -f $(PROJECT_NAME).bin
	rm -f $(PROJECT_NAME).map
	rm -f $(PROJECT_NAME).lst

reallyclean: clean
	rm -f $(LIB_OBJS) $(STD_PERIPH_LIB)/libstm32f0.a

remake: clean all
