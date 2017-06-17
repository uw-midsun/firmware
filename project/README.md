# Projects

Projects are stored in this directory. Each folder represents an individual project, identified by the folder's name and the existance of a `rules.mk` in the folder's root.

This is an example of a standard project's structure.
````
plutus
├── inc
│   └── crc15.h
├── rules.mk
├── src
│   ├── crc15.c
│   └── main.c
└── test
    └── test_crc15.c

3 directories, 5 files
````

## src/inc

`src` and `inc` are the default locations for source and header files respectively. The build system expects a flat folder. That is, it will not search for files recursively unless redefined in `rules.mk`.

To support platform-specific files, we define platform folders. Currently, those are `stm32f0xx` and `x86`. The build system expects platform folders to be either `src/$(PLATFORM)` or `inc/$(PLATFORM)`. Files in these platform-specific folders will only be included when that platform is built.

## test

Unit tests should be located in the `test` subdirectory. Tests are built using [Unity](https://github.com/ThrowTheSwitch/Unity) as a backend. Tests should be named `test_[module name].c` and include `setup_test(void)` and `teardown_test(void)`. `setup_test` is run before each test and `teardown_test` is run after each test. Each function `test_[module name]_[test name](void)` defines an individual unit test.

## rules.mk

`rules.mk` specifies a project's dependencies, flags, and allows for non-standard sources. The build system relies on the existence of `rules.mk` within a project's root to identify valid projects.

### Currently recognized variables

| Variable | Purpose | Default Value | Notes |
|----------|---------|---------------|-------|
| `$(T)_DEPS` | Define the libraries that this target depends on. | | A common depedency for projects is `ms-common`, which is our HAL. |
| `$(T)_SRC_ROOT` | Modify the source file (*.c, *.s) root directory | `$(T)_DIR/src` | Note that this must be the root folder. Only a single directory is valid. |
| `$(T)_INC_DIRS` | Modify the directories to search for headers in. | `$(T)_DIR/inc` | This may be any number of directories. |
| `$(T)_SRC` | Define the source files to compile. | `$(T)_DIR/src/[$(PLATFORM)/]*.(c,s)` | |
| `$(T)_INC` | Define the headers that should be watched. | `$(T)_DIR/inc/*.h` | |
| `$(T)_CFLAGS` | Define custom target CFLAGS. | `$(CFLAGS)` | |
| `$(T)_EXCLUDE_TESTS` | Define any tests that should be excluded. | | When specifying tests to exclude, do not include `test_`. |
