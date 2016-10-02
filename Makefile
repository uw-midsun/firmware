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
RULES_DIR := projects/test_project
include $(RULES_DIR)/rules.mk
MAIN_FILE = $(RULES_DIR)/$(MAIN)

# compile directory
BIN_DIR := bin

# location of OpenOCD board .cfg files (only triggered if you use 'make program' explicitly)
OPENOCD_BOARD_DIR := /usr/share/openocd/scripts/board

# Please don't touch anything below this line
###################################################################################################

# AUTOMATED ACTIONS

# name of generated *.elf file 
MAIN_PROJECT := $(basename $(notdir $(MAIN_FILE)))

# location of the libraries
LIB_DIR := libraries/$(DEVICE_FAMILY)
DEV_DIR := device/$(DEVICE_FAMILY)
MSLIB_DIR := libraries/ms-lib

###################################################################################################

# ENV SETUP

ROOT=$(shell pwd)

###################################################################################################

# MAKE RULES

.PHONY: lint proj program

all: $(LIB_DIR)/$(DEVICE_LIBRARY) $(MSLIB_DIR)/mslib.a lint project

include $(MSLIB_DIR)/mslib.mk
include $(DEV_DIR)/cfg.mk

lint:
	@-find projects -name "*.c" -o -name "*.h" | xargs -r python2 lint.py
	@-find $(MSLIB_DIR) -name "*.c" -o -name "*.h" | xargs -r python2 lint.py

###################################################################################################

# OPENOCD SUPPORT

# TODO verify this isn't broken
program: $(foreach project,$(PROJECT_NAME),$(BIN)/$(project).bin)

$(BIN_DIR)/%.bin: $(BIN_DIR)/%.bin
	openocd -f $(OPENOCD_BOARD_DIR)/board.cfg -f $(OPENOCD_CFG) \
	-c "stm_flash `pwd`/$@" -c shutdown

###################################################################################################

# EXTRA

# clean and remake rules, use reallyclean to remake the STD_PERIPH_LIB or before a push 

clean:
	@find ./ -name '*~' | xargs rm -f
	@rm -rf bin/

reallyclean: clean
	@rm -rf $(LIB_DIR)/obj $(LIB_DIR)/*.a
	@rm -rf $(MSLIB_DIR)/obj $(MSLIB_DIR)/*.a

remake: clean all
