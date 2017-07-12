#!/usr/bin/env python3
from string import Template
import os

rules = Template("""# Defines $$(T)_SRC, $$(T)_INC, $$(T)_DEPS, and $$(T)_CFLAGS for the build makefile.
# Tests can be excluded by defining $$(T)_EXCLUDE_TESTS.
# Pre-defined:
# $$(T)_SRC_ROOT: $$(T)_DIR/src
# $$(T)_INC_DIRS: $$(T)_DIR/inc{/$$(PLATFORM)}
# $$(T)_SRC: $$(T)_DIR/src{/$$(PLATFORM)}/*.{c,s}

# Specify the libraries you want to include
$$(T)_DEPS := $deps
""")

def new_target(target_type, name):
  type_folders = {
    'project': 'projects',
    'library': 'libraries'
  }

  proj_path = os.path.join(type_folders[target_type], name)
  folders = ['src', 'inc', 'test']

  for folder in folders:
    os.makedirs(os.path.join(proj_path, folder), exist_ok=True)

  deps = 'ms-common' if target_type == 'project' else ''

  with open(os.path.join(proj_path, 'rules.mk'), 'w') as f:
    f.write(rules.substitute({'deps': deps}))

  print('Created new {0} {1}'.format(target_type, name))

if __name__ == '__main__':
  import argparse

  parser = argparse.ArgumentParser('Creates new project/library')
  parser.add_argument('type', choices=['project', 'library'])
  parser.add_argument('name')
  args = parser.parse_args()

  new_target(args.type, args.name)
