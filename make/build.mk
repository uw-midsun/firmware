# Default build makefile
# Define TARGET as the library or project name and TARGET_TYPE as either LIB or PROJ.
# Builds the appropriate object files and creates a static library, and if applicable, an application

# Alias target so we don't have super long variable names
T := $(TARGET)
ifeq (,$(filter LIB PROJ,$(TARGET_TYPE)))
  $(error Invalid build target type for $(TARGET))
endif

# Defines default variables and targets
$(T)_DIR := $($(TARGET_TYPE)_DIR)/$(T)

$(T)_SRC_ROOT := $($(T)_DIR)/src
$(T)_OBJ_ROOT := $(OBJ_CACHE)/$(T)
$(T)_INC_DIRS := $($(T)_DIR)/inc

$(T)_SRC := $(wildcard $($(T)_SRC_ROOT)/*.c) \
            $(wildcard $($(T)_SRC_ROOT)/$(PLATFORM)/*.c) \
            $(wildcard $($(T)_SRC_ROOT)/*.s)
$(T)_INC := $(wildcard $($(T)_INC_DIRS)/*.h)

$(T)_CFLAGS := $(CFLAGS)

# Include library variables - we expect to have $(T)_SRC, $(T)_INC, $(T)_DEPS, and $(T)_CFLAGS
include $($(T)_DIR)/rules.mk

# Define objects and include generated dependencies
# Note that without some very complex rules, we can only support one root source directory.
$(T)_OBJ := $($(T)_SRC:$($(T)_SRC_ROOT)/%.c=$($(T)_OBJ_ROOT)/%.o)
$(T)_OBJ := $($(T)_OBJ:$($(T)_SRC_ROOT)/%.s=$($(T)_OBJ_ROOT)/%.o)
-include $($(T)_OBJ:.o=.d) #:

# Static library link target
$(STATIC_LIB_DIR)/lib$(T).a: $($(T)_OBJ) $(call dep_to_lib,$($(T)_DEPS)) | $(STATIC_LIB_DIR)
	@echo "Linking $@"
	@$(AR) -r $@ $^

# Application target
$(BIN_DIR)/$(T)$(PLATFORM_EXT): $($(T)_OBJ) $(call dep_to_lib,$($(T)_DEPS)) | $(BIN_DIR)
	@echo "Building $(notdir $@) for $(PLATFORM)"
	@$(CC) $(CFLAGS) -Wl,-Map=$(BIN_DIR)/$(notdir $@).map $^ -o $@ -L$(STATIC_LIB_DIR) \
		$(addprefix -l,$(APP_DEPS)) \
		$(LDFLAGS) $(addprefix -I,$(INC_DIRS))
	@$(OBJDUMP) -St $@ >$(basename $@).lst
	@$(SIZE) $@

# Object target - use first order-only dependency to expand the library name for subshells
$($(T)_OBJ_ROOT)/%.o: $($(T)_SRC_ROOT)/%.c | $(T) $(dir $($(T)_OBJ))
	@echo "$(firstword $|): $(notdir $<) -> $(notdir $@)"
	@$(CC) -MD -MP -w -c -o $@ $< $($(firstword $|)_CFLAGS) $(addprefix -I,$(INC_DIRS))

$($(T)_OBJ_ROOT)/%.o: $($(T)_SRC_ROOT)/%.s | $(T) $(dir $($(T)_OBJ))
	@echo "$(firstword $|): $(notdir $<) -> $(notdir $@)"
	@$(CC) -MD -MP -w -c -o $@ $< $($(firstword $|)_CFLAGS) $(addprefix -I,$(INC_DIRS))

.PHONY: $(T) $(TARGET_TYPE)
$(T): | $(TARGET_TYPE)
	$(eval APP_DEPS += $($(@)_DEPS))
	@echo "Processing $(firstword $|) $@"

$(TARGET_TYPE):

ifneq (unity,$(T))
  include $(MAKE_DIR)/build_test.mk
endif

DIRS := $(sort $(DIRS) $($(T)_OBJ_DIR) $(dir $($(T)_OBJ)))
INC_DIRS := $(sort $(INC_DIRS) $(dir $($(T)_INC)))
