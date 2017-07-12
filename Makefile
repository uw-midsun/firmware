###################################################################################################
# Midnight Sun's build system
#
# Arguments:
#		PL: [PLATFORM=] - Specifies the target platform (based on device family, defaults to stm32f0xx)
#		PR: [PROJECT=] - Specifies the target project
#		LI: [LIBRARY=] - Specifies the target library (only valid for tests)
#		TE: [TEST=] - Specifies the target test (only valid for tests, requires LI or PR to be specified)
#		CM: [COMPILER=] - Specifies the compiler to use on x86. Defaults to gcc [gcc | clang].
#		CO: [COPTIONS=] - Specifies compiler options on x86 [asan | tsan].
#   PB: [PROBE=] - Specifies which debug probe to use on STM32F0xx. Defaults to cmsis-dap [cmsis-dap | stlink-v2].
#
# Usage:
#		make [all] [PL] [PR] - Builds the target project and its dependencies
#		make clean [PL] [PR] - Removes the project's build output and associated linker and object files
#		make remake [PL] [PR] - Cleans and rebuilds the target project (does not force-rebuild dependencies)
#		make reallyclean - Completely deletes all build output
#   make new [PR|LI] - Creates folder structure for new project or library
#		make lint - Lints all non-vendor code
#		make test [PL] [PR|LI] [TE] - Builds and runs the specified unit test, assuming all tests if TE is not defined
#		make gdb [PL] [PR] - Builds and runs the project and connects an instance of GDB for debugging
#   make gdb [PL] [PR|LI] [TE] - Builds and runs the specified unit test and connects an instance of GDB
# Platform specific:
#		make program [PL=stm32f0xx] [PR] [PB] - Programs and runs the project through OpenOCD
#	  make gdb [PL=stm32f0xx] [PL] [PR] [PB]
#		make <build | test | remake | all> [PL=x86] [CM=clang [CO]]
#
###################################################################################################

# CONFIG

# Default directories
PROJ_DIR := projects
PLATFORMS_DIR := platform
LIB_DIR := libraries
MAKE_DIR := make

PLATFORM := stm32f0xx

# Include argument filters
include $(MAKE_DIR)/filter.mk

# Location of project
PROJECT_DIR := $(PROJ_DIR)/$(PROJECT)

# Location of platform
PLATFORM_DIR := $(PLATFORMS_DIR)/$(PLATFORM)

# Output directory
BUILD_DIR := build

# compile directory
BIN_DIR := $(BUILD_DIR)/bin/$(PLATFORM)

# Static library directory
STATIC_LIB_DIR := $(BUILD_DIR)/lib/$(PLATFORM)

# Object cache
OBJ_CACHE := $(BUILD_DIR)/obj/$(PLATFORM)

# Set GDB target
ifeq (,$(TEST))
GDB_TARGET = $(BIN_DIR)/$(PROJECT)$(PLATFORM_EXT)
else
GDB_TARGET = $(BIN_DIR)/test/$(LIBRARY)$(PROJECT)/test_$(TEST)_runner$(PLATFORM_EXT)
endif

DIRS := $(BUILD_DIR) $(BIN_DIR) $(STATIC_LIB_DIR) $(OBJ_CACHE)

# Please don't touch anything below this line
###################################################################################################

# AUTOMATED ACTIONS

# $(call include_lib,libname)
define include_lib
$(eval TARGET := $(1));
$(eval TARGET_TYPE := LIB);
$(eval include $(MAKE_DIR)/build.mk);
$(eval undefine TARGET; undefine TARGET_TYPE)
endef

# $(call include_proj,projname)
define include_proj
$(eval TARGET := $(1));
$(eval TARGET_TYPE := PROJ);
$(eval include $(MAKE_DIR)/build.mk);
$(eval undefine TARGET; undefine TARGET_TYPE)
endef

# $(call dep_to_lib,deps)
define dep_to_lib
$(1:%=$(STATIC_LIB_DIR)/lib%.a)
endef
.PHONY: # Just adding a colon to fix syntax highlighting

# $(call find_in,folders,wildcard)
define find_in
$(foreach folder,$(1),$(wildcard $(folder)/$(2)))
endef

# include the target build rules
-include $(PROJECT_DIR)/rules.mk

###################################################################################################

# ENV SETUP

ROOT := $(shell pwd)

###################################################################################################

# MAKE PROJECT

.PHONY: all lint project build_all

# Actually calls the make
all: build lint

# Includes platform-specific configurations
include $(PLATFORMS_DIR)/$(PLATFORM)/platform.mk

# Includes all libraries so make can find their targets
$(foreach lib,$(VALID_LIBRARIES),$(call include_lib,$(lib)))

# Includes all projects so make can find their targets
$(foreach proj,$(VALID_PROJECTS),$(call include_proj,$(proj)))

# Lints the files in ms-common and projects
lint:
	@find $(PROJ_DIR) -name "*.c" -o -name "*.h" | xargs -P 24 -r python2 lint.py
	@find "$(LIB_DIR)/ms-common" -name "*.c" -o -name "*.h" | xargs -P 24 -r python2 lint.py

# Builds the project
build: $(BIN_DIR)/$(PROJECT)$(PLATFORM_EXT)

build_all: $(VALID_PROJECTS:%=$(BIN_DIR)/%$(PLATFORM_EXT))

$(DIRS):
	@mkdir -p $@

$(BIN_DIR)/%.bin: $(BIN_DIR)/%$(PLATFORM_EXT)
	@$(OBJCPY) -O binary $< $(<:$(PLATFORM_EXT)=.bin)

###################################################################################################

# EXTRA

# clean and remake rules, use reallyclean to clean everything

.PHONY: clean reallyclean remake new

new:
	@python3 $(MAKE_DIR)/new_project.py $(NEW_TYPE) $(PROJECT)$(LIBRARY)

clean:
	@find ./ -name '*~' | xargs rm -f
	@rm -rf $(BIN_DIR)

reallyclean: clean
	@rm -rf $(BUILD_DIR)

remake: clean all
