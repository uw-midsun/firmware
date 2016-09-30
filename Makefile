###################################################################################################
# AUTHOR: Calder Kitagawa
# PURPOSE: CORTEX M0 package template using stm32f0xx libraries
# DATE: SEPT 24 2016
# MODIFIED:
# VERSION: 1.0.0 
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

# put the root direcories you want to compile here separated by a space
_SRCS = src

# include directories
_INC = inc 

# compile directory
BIN = bin

# the name you want the generated .elf file to go by should go here
PROJECT_NAME=$(basename $(SRCS))

# location of the libraries
STD_PERIPH_LIB=libraries/stm32f0xx

# linker scripts location if you move the linkers
# DO NOT TOUCH UNLESS YOU ABSOLUTELY KNOW WHAT YOU ARE DOING
LDSCRIPT_INC=device/stm32f0xx/ldscripts

###################################################################################################

# OPENOCD SSUPPORT

# location of OpenOCD board .cfg files (only triggered if you use 'make program' explicitly)
OPENOCD_BOARD_DIR=/usr/share/openocd/scripts/board

# configuration (cfg) file containing directives for OpenOCD
OPENOCD_PROC_FILE=$(STD_PERIPH_LIB)/stm32f0-opencd.cfg

# the rest of this is taken care of please don't touch anything else
###################################################################################################

# GENERAL COMPILER SETUP

# Use gcc-arm for everything
CC=arm-none-eabi-gcc
OBJCPY=arm-none-eabi-objcopy
OBJDUMP=arm-none-eabi-objdump
SIZE=arm-none-eabi-size
AR=arm-none-eabi-ar

# TODO verify this is the correct architecture
ARCH_FLAGS = -mlittle-endian -mcpu=cortex-m0 -march=armv6-m -mthumb
INC_FLAGS = -include $(STD_PERIPH_LIB)/stm32f0xx_conf.h \
		-isystem $(STD_PERIPH_LIB)/CMSIS/Include \
		-isystem $(STD_PERIPH_LIB)/CMSIS/Device/ST/STM32F0xx/Include \
		-isystem $(STD_PERIPH_LIB)/STM32F0xx_StdPeriph_Driver/inc \
		-isystem $(STD_PERIPH_LIB)/STM32F0xx_StdPeriph_Driver/src

CFLAGS = -Wall -Werror -g -Os -Wno-unused-variable -pedantic
CFLAGS += $(ARCH_FLAGS)
CFLAGS += -ffunction-sections -fdata-sections
CFLAGS += -Wl,--gc-sections #-Wl,-Map=$(PROJECT_NAME).map

###################################################################################################
# TEMPLATE SETUP

vpath %.c src
vpath %.a $(STD_PERIPH_LIB)

ROOT=$(shell pwd)

INC = $(addprefix -I, $(_INC))

SRCS = $(notdir $(wildcard $(_SRCS)/*.c)) 

# TODO Investigating making this more dynamic
CFLAGS += $(INC) -isystem $(STD_PERIPH_LIB) $(INC_FLAGS) 

# Include the startup file in the make
STARTUP = device/stm32f0xx/startup_stm32f0xx.s

OBJS = $(SRCS:.c=.o)

###################################################################################################

# STM32F0xx LIB SETUP

# Use separate flags to build the STD_PERIPH_LIB
LIB_CFLAGS = -g -O2 -Wall
LIB_CFLAGS += $(ARCH_FLAGS)
LIB_CFLAGS += -ffreestanding -nostdlib $(INC_FLAGS)

# STD_PERIPH_LIB source files
LIB_SRCS = $(notdir $(wildcard $(STD_PERIPH_LIB)/STM32F0xx_StdPeriph_Driver/src/*.c))

# Store object files in a obj directory
LIB_OBJS = $(addprefix $(STD_PERIPH_LIB)/obj/, $(LIB_SRCS:.c=.o))

###################################################################################################

# MAKE RULES

.PHONY: lint proj program

all: $(STD_PERIPH_LIB)/libstm32f0.a lint proj

lint:
	@-find inc -name "*.c" -o -name "*.h" | xargs -r python2 cpplint.py
	@-find src -name "*.c" -o -name "*.h" | xargs -r python2 cpplint.py

# compiles library objects
$(STD_PERIPH_LIB)/obj/%.o : $(STD_PERIPH_LIB)/STM32F0xx_StdPeriph_Driver/src/%.c
	@mkdir -p $(STD_PERIPH_LIB)/obj/
	@$(CC) -w -c -o $@ $< $(LIB_CFLAGS) 

# builds the library
$(STD_PERIPH_LIB)/libstm32f0.a: $(LIB_OBJS)
	@$(AR) -r $@ $(LIB_OBJS)

# call to build the project
proj:   $(foreach project,$(PROJECT_NAME),$(BIN)/$(project).elf)

# This is what actually builds the project
# The necessity of an OBJDUMP and SIZE file for debug is questionable as it is only
# a feature for power-users who would know how to enable it anyway
$(BIN)/%.elf: %.c $(STARTUP)
	@mkdir -p bin/	
	@$(CC) $(CFLAGS) -Wl,-Map=$(basename $@).map $^ -o $@ \
		-L$(STD_PERIPH_LIB) -lstm32f0 -L$(LDSCRIPT_INC) -Tstm32f0.ld
	@$(OBJCPY) -O binary $@ $(basename $@).bin
	@$(OBJDUMP) -St $@ >$(basename $@).lst
	$(SIZE) $@

# OPTIONAL: add this line to the above rule to compile a hex file as well
# $(OBJCOPY) -O ihex $(PROJ_NAME).elf $(PROJ_NAME).hex

###################################################################################################

# OPENOCD SUPPORT

# TODO verify this isn't broken
program: $(foreach project,$(PROJECT_NAME),$(BIN)/$(project).bin)

$(BIN)/%.bin: $(BIN)/%.bin
	openocd -f $(OPENOCD_BOARD_DIR)/stm32f0discovery.cfg -f $(OPENOCD_PROC_FILE) \
	-c "stm_flash `pwd`/$@" -c shutdown

###################################################################################################

# EXTRA

# clean and remake rules, use reallyclean to remake the STD_PERIPH_LIB or before a push 

clean:
	@find ./ -name '*~' | xargs rm -f
	@rm -rf bin/

reallyclean: clean
	@rm -rf $(STD_PERIPH_LIB)/obj $(STD_PERIPH_LIB)/libstm32f0.a

remake: clean all
