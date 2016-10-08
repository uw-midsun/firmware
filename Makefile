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

# Specify the directory for the project you want to build. Must contain a rules.mk defining
#
# Default (for now) pass PROJECT=<path-to-project> as an argument to override
PROJECT := projects/test_project

# Default (for now) pass DEVICE_FAMILY=<device> as an argument to override
DEVICE_FAMILY := stm32f0xx

# Output directory
BUILD_DIR := build

# compile directory
BIN_DIR := $(BUILD_DIR)/bin

# Static library directory
STATIC_LIB_DIR := $(BUILD_DIR)/lib

# Object cache
OBJ_CACHE := $(BUILD_DIR)/obj

DIRS := $(BUILD_DIR) $(BIN_DIR) $(STATIC_LIB_DIR) $(OBJ_CACHE)

LIB_DIR := libraries

# location of OpenOCD board .cfg files (only triggered if you use 'make program' explicitly)
OPENOCD_BOARD_DIR := /usr/share/openocd/scripts/board

# Please don't touch anything below this line
###################################################################################################

# AUTOMATED ACTIONS

# $(call include_lib,libname,dep_var)
# Assumes the existance of $($(LIB)_OBJ_DIR)
define include_lib
$(eval LIB := $(1));
$(eval include $(LIB_DIR)/$(1)/rules.mk);
$(eval DIRS := $(sort $(DIRS) $($(LIB)_OBJ_DIR) $(dir $($(LIB)_OBJ))));
$(eval undefine LIB)
endef

# $(call dep_to_lib,deps)
define dep_to_lib
$(1:%=$(STATIC_LIB_DIR)/lib%.a)
endef
.PHONY: # need colon to fix syntax highlighting

# include the target build rules
include $(PROJECT)/rules.mk

# define a MAIN_FILE and PROJECT_NAME using the rules included in the last section
MAIN_FILE := $(PROJECT)/$(MAIN)
PROJECT_NAME := $(basename $(notdir $(MAIN_FILE)))

# Find all libraries available
LIBS := $(patsubst $(LIB_DIR)/%/rules.mk,%,$(wildcard $(LIB_DIR)/*/rules.mk))

# Define the static libraries required for the project
APP_LIBS := $(call dep_to_lib,$(APP_DEPS))

###################################################################################################

# ENV SETUP

ROOT := $(shell pwd)

###################################################################################################

# MAKE PROJECT

.PHONY: all lint proj program

# Actually calls the make
all: project lint

# Includes device specific configurations
include device/$(DEVICE_FAMILY)/device_config.mk

# Includes all libraries so make can find their targets
$(foreach dep,$(LIBS),$(call include_lib,$(dep)))

# Lints the files in ms-lib and projects
lint:
	@-find projects -name "*.c" -o -name "*.h" | xargs -P 24 -r python2 lint.py
	@-find libraries -path "$(LIB_DIR)/stm32f0xx" -prune -o -name "*.c" -o -name "*.h" | xargs -P 24 -r python2 lint.py

# Builds the project
project: $(BIN_DIR)/$(PROJECT_NAME).elf

# Rule for making the project
$(BIN_DIR)/%.elf: $(MAIN_FILE) $(HEADERS) $(STARTUP) $(APP_LIBS) | $(BIN_DIR)
	@$(CC) $(CFLAGS) $^ -o $@ -L$(STATIC_LIB_DIR)\
		$(foreach dep,$(APP_DEPS), -l$(notdir $(dep))) \
		$(LINKER)
	@$(OBJCPY) -O binary $@ $(BIN_DIR)/$(PROJECT_NAME).bin
	@$(OBJDUMP) -St $@ >$(basename $@).lst
	$(SIZE) $@

$(DIRS):
	@mkdir -p $@

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
	@rm -rf $(BUILD_DIR)

remake: clean all
