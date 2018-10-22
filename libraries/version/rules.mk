# Defines $(T)_SRC, $(T)_INC, $(T)_DEPS, and $(T)_CFLAGS for the build makefile.
# Tests can be excluded by defining $(T)_EXCLUDE_TESTS.
# Pre-defined:
# $(T)_SRC_ROOT: $(T)_DIR/src
# $(T)_INC_DIRS: $(T)_DIR/inc{/$(PLATFORM)}
# $(T)_SRC: $(T)_DIR/src{/$(PLATFORM)}/*.{c,s}

$(T)_DEPS := ms-common

# GIT_VERSION_COMMIT_HASH is an 8-digit git commit hash
GIT_VERSION_COMMIT_HASH := $(shell git rev-parse --short HEAD)
# GIT_VERSION_DIRTY_STATUS denotes whether or not the tree is dirty
GIT_VERSION_DIRTY_STATUS := "clean"

# We define a tree as "dirty" if there exist any possible changes that might
# affect our build outputs. Formally, the tree is considered "dirty" if there
# are any:
#
# 1. Staged changes
# 2. Unstaged changes
# 3. Untracked changes in files that are not ignored
#
# This is so we don't need to parse the dependencies of each project and their
# transitive dependencies.
#
# Theoretically, we should just be able to get away with running the porcelain
# version of git status. Otherwise, we can do the same thing, but with plumbing
# commands.
#
# Staged changes
# git diff-index --quiet --cached HEAD --
# Return:
#  - 0 if no changes
#  - 1 if changes
#
# Unstaged changes:
# git diff-files --quiet
# Return:
#  - 0 if no unstaged changes
#  - 1 if unstaged changes
#
# Untracked changes
#  git ls-files --exclude-standard --others .
#
# For the sake of simplicity, we're just going to use the porcelain commands,
# and if we run into issues down the road, we know what to fix.
ifneq ($(strip $(shell git status --porcelain 2>/dev/null)),)
  GIT_VERSION_DIRTY_STATUS := "dirty"
endif

# Add variables
$(T)_CFLAGS += -DGIT_VERSION_COMMIT_HASH=\"$(GIT_VERSION_COMMIT_HASH)\" \
                -DGIT_VERSION_DIRTY_STATUS=\"$(GIT_VERSION_DIRTY_STATUS)\"

# Force the dependency to be rebuilt when the CFLAGS variables are updated
$($(T)_SRC_ROOT)/git_version.c: .FORCE
	@touch $@
	@echo "Version: $(GIT_VERSION_COMMIT_HASH): $(GIT_VERSION_DIRTY_STATUS)"
