# Unity
UNITY_ROOT := $(LIB_DIR)/unity
UNITY_SCRIPT_DIR := $(UNITY_ROOT)/auto
UNITY_GEN_RUNNER := ruby $(UNITY_SCRIPT_DIR)/generate_test_runner.rb --setup_name=setup_test --teardown_name=teardown_test

# Test directories
$(T)_GEN_DIR := $(BUILD_DIR)/gen/$(PLATFORM)
$(T)_TEST_BIN_DIR := $(BIN_DIR)/test/$(T)
$(T)_TEST_OBJ_DIR := $($(T)_OBJ_ROOT)/test
$(T)_TEST_ROOT := $($(T)_DIR)/test

$(T)_TEST_DEPS := unity $(T)

# Find all test_*.c files - these are our unit tests
$(T)_TEST_SRC := $(wildcard $($(T)_TEST_ROOT)/test_*.c)
$(T)_TEST_OBJ := $($(T)_TEST_SRC:$($(T)_TEST_ROOT)/%.c=$($(T)_TEST_OBJ_DIR)/%.o)
-include $($(T)_TEST_OBJ:.o=.d) #:

# Generate the appropriate test runners for our unit tests
$(T)_TEST_RUNNERS := $($(T)_TEST_SRC:$($(T)_TEST_ROOT)/test_%.c=$($(T)_GEN_DIR)/test_%_runner.c)
$(T)_TEST_RUNNERS_OBJ := $($(T)_TEST_RUNNERS:$($(T)_GEN_DIR)/%.c=$($(T)_TEST_OBJ_DIR/%.o))
-include $($(T)_TEST_RUNNERS_OBJ:.o=.d) #:

.SECONDARY: $($(T)_TEST_RUNNERS)

# Generate the expected build outputs - one for each runner
$(T)_TESTS := $($(T)_TEST_RUNNERS:$($(T)_GEN_DIR)/%.c=$($(T)_TEST_BIN_DIR)/%$(PLATFORM_EXT))

# Generate the test runners
$($(T)_GEN_DIR)/%_runner.c: $($(T)_TEST_ROOT)/%.c | $($(T)_GEN_DIR)
	@echo "Generating $(notdir $@)"
	@$(UNITY_GEN_RUNNER) $< $@

# Compile the unit tests
$($(T)_TEST_OBJ_DIR)/%.o: $($(T)_TEST_ROOT)/%.c | $(T) $(dir $($(T)_TEST_OBJ))
	@echo "T: $(notdir $<) -> $(notdir $@)"
	@$(CC) -MD -MP -w -c -o $@ $< $($(firstword $|)_CFLAGS) $(addprefix -I,$(INC_DIRS))

# Compile the test runners
$($(T)_TEST_OBJ_DIR)/%.o: $($(T)_GEN_DIR)/%.c | $(T) $(dir $($(T)_TEST_RUNNERS_OBJ))
	@echo "T: $(notdir $<) -> $(notdir $@)"
	@$(CC) -MD -MP -w -c -o $@ $< $($(firstword $|)_CFLAGS) $(addprefix -I,$(INC_DIRS))

# Build each test - only include the test's runner and unit tests.
$($(T)_TESTS): $($(T)_TEST_BIN_DIR)/%_runner$(PLATFORM_EXT): \
                 $($(T)_TEST_OBJ_DIR)/%.o $($(T)_TEST_OBJ_DIR)/%_runner.o \
                 $(call dep_to_lib,$($(T)_TEST_DEPS)) | $($(T)_TEST_BIN_DIR)
	@echo "Building test $(notdir $@) for $(PLATFORM)"
	@$(CC) $(CFLAGS) -Wl,-Map=$|/$(notdir $(@:%$(PLATFORM_EXT)=%.map)) $^ -o $@ -L$(STATIC_LIB_DIR) \
		$(addprefix -l,$(APP_DEPS)) \
		$(LDFLAGS) $(addprefix -I,$(INC_DIRS))

.PHONY: test test_ test_$(T)

ifeq ($(T),$(filter $(T),$(LIBRARY) $(PROJECT)))
ifeq (,$(TEST))
test: test_$(T)
else
test: $($(T)_TEST_BIN_DIR)/test_$(TEST)_runner$(PLATFORM_EXT)
	@echo "Running test - $(TEST)"
	@$(call run_test,$<)

endif
endif

# Run each test
test_$(T): $($(T)_TESTS)
	@echo "Running test suite - $(@:test_%=%)"
	@$(foreach test,$^,$(call run_test,$(test)) &&) true

test_all: test_$(T)

build_all: $($(T)_TESTS)

DIRS := $(sort $(DIRS) $($(T)_GEN_DIR) $($(T)_TEST_BIN_DIR) \
               $(dir $($(T)_TEST_OBJ) $($(T)_TEST_RUNNERS_OBJ)))
