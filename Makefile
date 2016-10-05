###################################################################################################
# AUTHOR: Calder Kitagawa
# PURPOSE: CORTEX M0 package template using stm32f0xx libraries
# DATE: SEPT 24 2016
# MODIFIED:
# VERSION: 1.0.0 
#
# USAGE:
# make [all] - makes the device/mslib libraries if not cached and makes the target ruleset 
#	make remake - rebuilds .elf 
#	make clean - removes the .elf and associated linker and object files
#	make reallyclean - in addition to running make clean also removed the cached libraries
#	make program - builds an OpenOCD binary
#
###################################################################################################

# CONFIG

# Specify the directory with the rule.mk file for the project you want to build
# Default (for now) pass RULES=<path-to-rules> as an argument to override
RULES := projects/test_project

# Default (for now) pass DEVICE_FAMILY=<device> as an argument to override
DEVICE_FAMILY := stm32f0xx

# compile directory
BIN_DIR := bin

# library cache
OBJ_CACHE := obj

# location of OpenOCD board .cfg files (only triggered if you use 'make program' explicitly)
OPENOCD_BOARD_DIR := /usr/share/openocd/scripts/board

# Please don't touch anything below this line
###################################################################################################

# AUTOMATED ACTIONS

# include the target build rules
include $(RULES)/rules.mk

# define a MAIN_FILE and PROJECT_NAME using the rules included in the last section
MAIN_FILE := $(RULES)/$(MAIN)
PROJECT_NAME := $(basename $(notdir $(MAIN_FILE)))

# string manipulations to define the required libraries based on the DEPS variable in the RULES 
_DEPS := $(foreach dep,$(DEPS),$(addprefix libraries/,$(addsuffix /$(dep),$(dep))))
APP_DEPS := $(addprefix $(OBJ_CACHE)/lib,$(notdir $(addsuffix .a,$(_DEPS))))

###################################################################################################

# ENV SETUP

ROOT=$(shell pwd)

###################################################################################################

# MAKE RULES

.PHONY: all lint proj program

# Actually calls the make
all: $(APP_DEPS) lint project

# Includes device specific configurations
include device/$(DEVICE_FAMILY)/device_config.mk

# Includes libraries needed using LIB_DIR to make the rules.mk for each standardized
$(foreach dep,$(_DEPS),$(eval override LIB_DIR := $(notdir $(dep))) $(eval include $(dir $(dep))rules.mk))

# Lints the files in ms-lib and projects
lint:
	@-find projects -name "*.c" -o -name "*.h" | xargs -r python2 lint.py
	@-find libraries/ms-lib -name "*.c" -o -name "*.h" | xargs -r python2 lint.py 

# Builds the project
project: $(BIN_DIR)/$(PROJECT_NAME).elf

# Rule for making the project
$(BIN_DIR)/%.elf: $(MAIN_FILE) $(HEADERS) $(STARTUP)
	@mkdir -p $(BIN_DIR)
	@$(CC) $(CFLAGS) $^ -o $@ -L$(OBJ_CACHE)\
		$(foreach dep,$(DEPS), -l$(notdir $(dep))) \
		$(LINKER)
	@$(OBJCPY) -O binary $@ $(BIN_DIR)/$(PROJECT_NAME).bin
	@$(OBJDUMP) -St $@ >$(basename $@).lst
	$(SIZE) $@

# OPTIONAL:
# $(OBJCPY) -o ihex $(PROJECT_NAME).hex

###################################################################################################

# OPENOCD SUPPORT

# TODO verify this isn't broken
program: $(BIN_DIR)/$(PROJECT_NAME).bin

$(BIN_DIR)/%.bin: $(BIN_DIR)/%.bin
	openocd -f $(OPENOCD_BOARD_DIR)/board.cfg -f $(OPENOCD_CFG) \
	-c "stm_flash `pwd`/$@" -c shutdown

###################################################################################################

# EXTRA

# clean and remake rules, use reallyclean to remake the STD_PERIPH_LIB or before a push 

clean:
	@find ./ -name '*~' | xargs rm -f
	@rm -rf $(BIN_DIR)

reallyclean: clean
	@rm -rf $(OBJ_CACHE)
	@find libraries -type d -name 'obj' | xargs rm -rf

remake: clean all
