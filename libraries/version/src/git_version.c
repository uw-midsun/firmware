#include "git_version.h"

#include "git_version_impl.h"
#include "log.h"

static const char s_commit_hash[] = GIT_VERSION_COMMIT_HASH;
static const char s_dirty_status[] = GIT_VERSION_DIRTY_STATUS;

void git_version_init(void) {
  LOG_DEBUG("Version: %s-%s\n", s_commit_hash, s_dirty_status);
}
