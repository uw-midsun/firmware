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

# Specify the device to build for
devicefamily = stm32f0xx

# Specify the mainprojectfile you want to build
mainprojectfile = projects/main.c 

# compile directory
bindir = bin

# location of OpenOCD board .cfg files (only triggered if you use 'make program' explicitly)
OPENOCD_BOARD_DIR=/usr/share/openocd/scripts/board

###################################################################################################

# AUTOMATED ACTIONS

# name of generated *.elf files 
mainprojectname = $(basename $(notdir $(mainprojectfile)))

# location of the libraries
lib_dir = libraries/$(devicefamily)
devdir = device/$(devicefamily)
mslibdir = libraries/ms-lib

# the rest of this is taken care of please don't touch anything else
###################################################################################################

# ENV SETUP

ROOT=$(shell pwd)

###################################################################################################

# MAKE RULES

.PHONY: lint proj program

all: $(lib_dir)/libstm32f0.a $(mslibdir)/mslib.a lint project

include $(mslibdir)/mslib.mk
include $(devdir)/$(devicefamily)cfg.mk

lint:
	@-find projects -name "*.c" -o -name "*.h" | xargs -r python2 lint.py
	@-find $(mslibdir) -name "*.c" -o -name "*.h" | xargs -r python2 lint.py

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
	@rm -rf $(lib_dir)/obj $(lib_dir)/*.a
	@rm -rf $(mslibdir)/obj $(mslibdir)/*.a

remake: clean all
