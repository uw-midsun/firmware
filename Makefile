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
PROJECTS_DIR := project
PLATFORMS_DIR := platform

VALID_PROJECTS := $(patsubst $(PROJECTS_DIR)/%/rules.mk,%,$(wildcard $(PROJECTS_DIR)/*/rules.mk))
VALID_PLATFORMS := $(patsubst $(PLATFORMS_DIR)/%/platform.mk,%,$(wildcard $(PLATFORMS_DIR)/*/platform.mk))

PROJECT := $(filter $(VALID_PROJECTS),$(PROJECT))

# TODO: allow valid platforms to be defined by projects?
PLATFORM := stm32f0xx
override PLATFORM := $(filter $(VALID_PLATFORMS),$(PLATFORM))

# Only ignore project and platform if we're doing a full clean or lint
ifeq (,$(filter reallyclean lint,$(MAKECMDGOALS)))
ifeq (,$(PROJECT))
  $(error Invalid project. Expected PROJECT=[$(VALID_PROJECTS)])
endif

ifeq (,$(PLATFORM))
  $(error Invalid platform. Expected PLATFORM=[$(VALID_PLATFORMS)])
endif
endif

# Location of project
PROJECT_DIR := $(PROJECTS_DIR)/$(PROJECT)

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

LIB_DIR := libraries

# Please don't touch anything below this line
###################################################################################################

# AUTOMATED ACTIONS

# $(call include_lib,libname)
define include_lib
$(eval LIB := $(1));
$(eval include $(LIB_DIR)/library.mk);
$(eval DIRS := $(sort $(DIRS) $($(LIB)_OBJ_DIR) $(dir $($(LIB)_OBJ))));
$(eval INC_DIRS := $(sort $(INC_DIRS) $(dir $($(LIB)_INC))));
$(eval undefine LIB)
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

# define a MAIN_FILE using the rules included in the last section
MAIN_FILE := $(PROJECT_DIR)/$(MAIN)

# Find all libraries available
LIBS := $(patsubst $(LIB_DIR)/%/rules.mk,%,$(wildcard $(LIB_DIR)/*/rules.mk))

# Define the static libraries required for the project
APP_LIBS := $(call dep_to_lib,$(APP_DEPS))

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
$(foreach dep,$(LIBS),$(call include_lib,$(dep)))

# Lints the files in ms-common and projects
lint:
	@find $(PROJECTS_DIR) -name "*.c" -o -name "*.h" | xargs -P 24 -r python2 lint.py
	@find "$(LIB_DIR)/ms-common" -name "*.c" -o -name "*.h" | xargs -P 24 -r python2 lint.py

# Builds the project
project: $(BIN_DIR)/$(PROJECT)$(PLATFORM_EXT)

# Rule for making the project
$(BIN_DIR)/%$(PLATFORM_EXT): $(MAIN_FILE) $(APP_LIBS) | $(BIN_DIR)
	@echo "Building $(notdir $@) for $(PLATFORM)"
	@$(CC) $(CFLAGS) $^ -o $@ -L$(STATIC_LIB_DIR) \
		$(addprefix -l,$(APP_DEPS)) \
		$(LDFLAGS) $(addprefix -I,$(INC_DIRS))
	@$(OBJDUMP) -St $@ >$(basename $@).lst
	@$(SIZE) $@

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
