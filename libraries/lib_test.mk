UNITY_ROOT := $(LIB_DIR)/unity
UNITY_SCRIPT_DIR := $(UNITY_ROOT)/auto
UNITY_GEN_RUNNER := ruby $(UNITY_SCRIPT_DIR)/generate_test_runner.rb --setup_name=setup_test --teardown_name=teardown_test
$(LIB)_GEN_DIR := $(BUILD_DIR)/gen/$(PLATFORM)
$(LIB)_TEST_BIN_DIR := $(BIN_DIR)/test/$(LIB)
$(LIB)_TEST_OBJ_DIR := $($(LIB)_OBJ_ROOT)/test
DIRS += $($(LIB)_GEN_DIR) $($(LIB)_TEST_BIN_DIR)

$(LIB)_TEST_DEPS := unity $(LIB)

$(LIB)_TEST_ROOT := $($(LIB)_DIR)/test
$(LIB)_TEST := $(wildcard $($(LIB)_TEST_ROOT)/*.c)
$(LIB)_TEST_OBJ := $($(LIB)_TEST:$($(LIB)_TEST_ROOT)/%.c=$($(LIB)_TEST_OBJ_DIR)/%.o)
-include $($(LIB)_TEST_OBJ:.o=.d) #:

$(LIB)_TEST_RUNNERS := $($(LIB)_TEST:$($(LIB)_TEST_ROOT)/test_%.c=$($(LIB)_GEN_DIR)/test_%_runner.c)
$(LIB)_TEST_RUNNERS_OBJ := $($(LIB)_TEST_RUNNERS:$($(LIB)_GEN_DIR)/%.c=$($(LIB)_TEST_OBJ_DIR/%.o))
-include $($(LIB)_TEST_RUNNERS_OBJ:.o=.d) #:

DIRS += $(dir $($(LIB)_TEST_OBJ) $($(LIB)_TEST_RUNNERS_OBJ))

$(LIB)_TESTS := $($(LIB)_TEST_RUNNERS:$($(LIB)_GEN_DIR)/%.c=$($(LIB)_TEST_BIN_DIR)/%$(PLATFORM_EXT))

$($(LIB)_GEN_DIR)/%_runner.c: $($(LIB)_TEST_ROOT)/%.c | $($(LIB)_GEN_DIR)
	@echo "Generating $(notdir $@)"
	@$(UNITY_GEN_RUNNER) $< $@

$($(LIB)_TEST_OBJ_DIR)/%.o: $($(LIB)_TEST_ROOT)/%.c | $(LIB) $(dir $($(LIB)_TEST_OBJ))
	@echo "t: $(notdir $<) -> $(notdir $@)"
	@$(CC) -MD -MP -w -c -o $@ $< $($(firstword $|)_CFLAGS) $(addprefix -I,$(INC_DIRS))

$($(LIB)_TEST_OBJ_DIR)/%.o: $($(LIB)_GEN_DIR)/%.c | $(LIB) $(dir $($(LIB)_TEST_RUNNERS_OBJ))
	@echo "t: $(notdir $<) -> $(notdir $@)"
	@$(CC) -MD -MP -w -c -o $@ $< $($(firstword $|)_CFLAGS) $(addprefix -I,$(INC_DIRS))

$($(LIB)_TESTS): $($(LIB)_TEST_BIN_DIR)/%_runner$(PLATFORM_EXT): $($(LIB)_TEST_OBJ_DIR)/%.o $($(LIB)_TEST_OBJ_DIR)/%_runner.o $(call dep_to_lib,$($(LIB)_TEST_DEPS)) | $($(LIB)_TEST_BIN_DIR)
	@echo "Building test $(notdir $@) for $(PLATFORM)"
	@$(CC) $(CFLAGS) $^ -o $@ -L$(STATIC_LIB_DIR) \
		$(addprefix -l,$(APP_DEPS)) \
		$(LDFLAGS) $(addprefix -I,$(INC_DIRS))

.PHONY: test test_$(LIB)
test: test_$(LIB)

test_$(LIB): $($(LIB)_TESTS)
	@$(foreach test,$^,./$(test) &&) true