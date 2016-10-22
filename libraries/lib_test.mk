# Unity
UNITY_ROOT := $(LIB_DIR)/unity
UNITY_SCRIPT_DIR := $(UNITY_ROOT)/auto
UNITY_GEN_RUNNER := ruby $(UNITY_SCRIPT_DIR)/generate_test_runner.rb --setup_name=setup_test --teardown_name=teardown_test

# Test directories
$(LIB)_GEN_DIR := $(BUILD_DIR)/gen/$(PLATFORM)
$(LIB)_TEST_BIN_DIR := $(BIN_DIR)/test/$(LIB)
$(LIB)_TEST_OBJ_DIR := $($(LIB)_OBJ_ROOT)/test
$(LIB)_TEST_ROOT := $($(LIB)_DIR)/test

$(LIB)_TEST_DEPS := unity $(LIB)

# Find all test_*.c files - these are our unit tests
$(LIB)_TEST_SRC := $(wildcard $($(LIB)_TEST_ROOT)/test_*.c)
$(LIB)_TEST_OBJ := $($(LIB)_TEST_SRC:$($(LIB)_TEST_ROOT)/%.c=$($(LIB)_TEST_OBJ_DIR)/%.o)
-include $($(LIB)_TEST_OBJ:.o=.d) #:

# Generate the appropriate test runners for our unit tests
$(LIB)_TEST_RUNNERS := $($(LIB)_TEST_SRC:$($(LIB)_TEST_ROOT)/test_%.c=$($(LIB)_GEN_DIR)/test_%_runner.c)
$(LIB)_TEST_RUNNERS_OBJ := $($(LIB)_TEST_RUNNERS:$($(LIB)_GEN_DIR)/%.c=$($(LIB)_TEST_OBJ_DIR/%.o))
-include $($(LIB)_TEST_RUNNERS_OBJ:.o=.d) #:

# Generate the expected build outputs - one for each runner
$(LIB)_TESTS := $($(LIB)_TEST_RUNNERS:$($(LIB)_GEN_DIR)/%.c=$($(LIB)_TEST_BIN_DIR)/%$(PLATFORM_EXT))

# Generate the test runners
$($(LIB)_GEN_DIR)/%_runner.c: $($(LIB)_TEST_ROOT)/%.c | $($(LIB)_GEN_DIR)
	@echo "Generating $(notdir $@)"
	@$(UNITY_GEN_RUNNER) $< $@

# Compile the unit tests
$($(LIB)_TEST_OBJ_DIR)/%.o: $($(LIB)_TEST_ROOT)/%.c | $(LIB) $(dir $($(LIB)_TEST_OBJ))
	@echo "T: $(notdir $<) -> $(notdir $@)"
	@$(CC) -MD -MP -w -c -o $@ $< $($(firstword $|)_CFLAGS) $(addprefix -I,$(INC_DIRS))

# Compile the test runners
$($(LIB)_TEST_OBJ_DIR)/%.o: $($(LIB)_GEN_DIR)/%.c | $(LIB) $(dir $($(LIB)_TEST_RUNNERS_OBJ))
	@echo "T: $(notdir $<) -> $(notdir $@)"
	@$(CC) -MD -MP -w -c -o $@ $< $($(firstword $|)_CFLAGS) $(addprefix -I,$(INC_DIRS))

# Build each test - only include the test's runner and unit tests.
$($(LIB)_TESTS): $($(LIB)_TEST_BIN_DIR)/%_runner$(PLATFORM_EXT): \
                 $($(LIB)_TEST_OBJ_DIR)/%.o $($(LIB)_TEST_OBJ_DIR)/%_runner.o \
                 $(call dep_to_lib,$($(LIB)_TEST_DEPS)) | $($(LIB)_TEST_BIN_DIR)
	@echo "Building test $(notdir $@) for $(PLATFORM)"
	@$(CC) $(CFLAGS) $^ -o $@ -L$(STATIC_LIB_DIR) \
		$(addprefix -l,$(APP_DEPS)) \
		$(LDFLAGS) $(addprefix -I,$(INC_DIRS))

.PHONY: test test_ test_$(LIB)
# Only include the library tests as a target if we aren't testing a project
ifeq (,$(PROJECT))
test: test_$(LIBRARY)
test_: # Fake target for unspecified tests
endif

# Run each test
test_$(LIB): $($(LIB)_TESTS)
	@echo "Running test suite - $(@:test_%=%)"
	@$(foreach test,$^,./$(test) &&) true

DIRS := $(sort $(DIRS) $($(LIB)_GEN_DIR) $($(LIB)_TEST_BIN_DIR) \
               $(dir $($(LIB)_TEST_OBJ) $($(LIB)_TEST_RUNNERS_OBJ)))
