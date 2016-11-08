###################################################################################################
# Midnight Sun's build system
#
# USAGE:
#		make [all] - builds the libraries if not cached and builds the target
#		make remake - rebuilds .elf
#		make clean - removes the .elf and associated linker and object files
#		make reallyclean - in addition to running make clean also removes the cached libraries
#		make program - builds an OpenOCD binary
#
###################################################################################################

# CONFIG

# Default directories
PROJ_DIR := project
PLATFORMS_DIR := platform
LIB_DIR := libraries
MAKE_DIR := make

VALID_PROJECTS := $(patsubst $(PROJ_DIR)/%/rules.mk,%,$(wildcard $(PROJ_DIR)/*/rules.mk))
VALID_PLATFORMS := $(patsubst $(PLATFORMS_DIR)/%/platform.mk,%,$(wildcard $(PLATFORMS_DIR)/*/platform.mk))
VALID_LIBRARIES := $(patsubst $(LIB_DIR)/%/rules.mk,%,$(wildcard $(LIB_DIR)/*/rules.mk))

PROJECT := $(filter $(VALID_PROJECTS),$(PROJECT))

# TODO: allow valid platforms to be defined by projects?
PLATFORM := stm32f0xx
override PLATFORM := $(filter $(VALID_PLATFORMS),$(PLATFORM))
override PROJECT := $(filter $(VALID_PROJECTS),$(PROJECT))
override LIBRARY := $(filter $(VALID_LIBRARIES),$(LIBRARY))

# Only ignore project and platform if we're doing a full clean or lint
ifeq (,$(filter reallyclean lint build_all,$(MAKECMDGOALS)))
ifeq (,$(filter test test_all,$(MAKECMDGOALS)))
ifeq (,$(PROJECT))
  $(error Invalid project. Expected PROJECT=[$(VALID_PROJECTS)])
endif
else
# If running a test, ensure we have a project or library
ifeq (,$(PROJECT)$(LIBRARY))
  $(error Invalid project or library. Expected PROJECT=[$(VALID_PROJECTS)] or LIBRARY=[$(VALID_LIBRARIES)])
endif
endif

ifeq (,$(PLATFORM))
  $(error Invalid platform. Expected PLATFORM=[$(VALID_PLATFORMS)])
endif
endif

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

.PHONY: all lint project program gdb

# Actually calls the make
all: project lint

# Includes platform-specific configurations
-include $(PLATFORMS_DIR)/$(PLATFORM)/platform.mk

# Includes all libraries so make can find their targets
$(foreach lib,$(VALID_LIBRARIES),$(call include_lib,$(lib)))

# Includes all projects so make can find their targets
$(foreach proj,$(VALID_PROJECTS),$(call include_proj,$(proj)))

# Lints the files in ms-common and projects
lint:
	@find $(PROJ_DIR) -name "*.c" -o -name "*.h" | xargs -P 24 -r python2 lint.py
	@find "$(LIB_DIR)/ms-common" -name "*.c" -o -name "*.h" | xargs -P 24 -r python2 lint.py

# Builds the project
project: $(BIN_DIR)/$(PROJECT)$(PLATFORM_EXT)

build_all: $(VALID_PROJECTS:%=$(BIN_DIR)/%$(PLATFORM_EXT)) test_all

$(DIRS):
	@mkdir -p $@

###################################################################################################

# OPENOCD SUPPORT

program: $(BIN_DIR)/$(PROJECT).bin
	@$(OPENOCD) $(OPENOCD_CFG) -c "stm_flash `pwd`/$<" -c shutdown

gdb: $(BIN_DIR)/$(PROJECT)$(PLATFORM_EXT)
	@$(OPENOCD) $(OPENOCD_CFG) > /dev/null 2>&1 &
	@$(GDB) $< -ex "set pagination off" -ex "target extended-remote :3333" -ex "monitor reset halt" \
             -ex "load" -ex "tb main" -ex "c"
	@pkill openocd

$(BIN_DIR)/%.bin: $(BIN_DIR)/%$(PLATFORM_EXT)
	@$(OBJCPY) -O binary $< $(BIN_DIR)/$(PROJECT).bin

###################################################################################################

# EXTRA

# clean and remake rules, use reallyclean to clean everything

clean:
	@find ./ -name '*~' | xargs rm -f
	@rm -rf $(BIN_DIR)

reallyclean: clean
	@rm -rf $(BUILD_DIR)

remake: clean all
