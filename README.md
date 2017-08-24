# firmware

[![Build Status](https://travis-ci.org/uw-midsun/firmware.svg?branch=master)](https://travis-ci.org/uw-midsun/firmware)

This repository contains all the latest firmware for the [University of Waterloo](https://uwaterloo.ca/)'s [Midnight Sun Solar Rayce Car](http://www.uwmidsun.com/) team.

## Building and Testing

```bash
git clone https://github.com/uw-midsun/firmware.git firmware
cd firmware
make build_all PLATFORM=x86
make build_all PLATFORM=stm32f0xx
make test_all PLATFORM=x86
```

### Common Commands

We use [GNU Make](https://www.gnu.org/software/make/manual/) for our build system. See [Managing Projects with GNU Make, 3.Xth Edition](http://wanderinghorse.net/computing/make/book/ManagingProjectsWithGNUMake-3.1.3.pdf) for a fantastic supplement to the manual.

Our commands are documented in the top level of the root [Makefile](https://github.com/uw-midsun/firmware/blob/master/Makefile). Note that commands such as `test` and `gdb` will automatically build the project if any changes have been made. You do not need to explicitly build projects except for [continuous integration](#continuous-integration).

#### Creating a new project or library

```bash
make new PROJECT=new_project_name
make new LIBRARY=new_library_name
```

#### Building and flashing a project (STM32 only)

```bash
# Defaults to CMSIS-DAP
make program PROJECT=test_project
# Use ST-LINK/V2 (discovery board)
make program PROJECT=plutus PROBE=stlink-v2
```

#### Running a test

```bash
# Defaults to PLATFORM=stm32f0xx
make test PROJECT=plutus
make test LIBRARY=ms-common
make test TEST=soft_timer LIBRARY=ms-common

# x86
make test PROJECT=plutus PLATFORM=x86
```

#### Debugging a project

```bash
# Defaults to PLATFORM=stm32f0xx
make gdb PROJECT=test_project
make gdb TEST=soft_timer LIBRARY=ms-common

# x86
make gdb PROJECT=test_project PLATFORM=x86
```

#### Formatting and linting

```bash
make format
make lint
```

More information on building and testing can be found in our [Makefile](Makefile) and our [platform build rules](platform).

### Optional x86 Extended Debugging

If you have Clang/LLVM/Bear installed and want to debug on x86 more easily/more in depth.

#### Static Analysis

To create a compile commands database, run

```bash
make reallyclean
bear make build_all PLATFORM=x86
```

Then to perform static analysis, run

```bash
clang-tidy $PATH_TO_C_FILE -checks=*
```

#### Address Sanitation, Memory Analysis and Stack Pointers

To build in debug with memory and address sanitation and extended stack traces on faults, run

```bash
make reallyclean
make build_all PLATFORM=x86 COMPILER=clang COPTIONS=asan
```

If you run any of the resulting binaries and a memory error of any kind occurs there will be detailed information on the cause.

#### Thread Sanitation

To build in debug with thread sanitation run

```bash
make reallyclean
make build_all PLATFORM=x86 COMPILER=clang COPTIONS=tsan
```

If you run any of the resulting binaries and there is any multithreaded code this will find any race conditions.

## Continuous Integration

We use [Travis CI](https://travis-ci.org/uw-midsun) to run our continuous integration tests, which consists of linting project code, and compiling and running unit tests against each supported platform. The build matrix is used to run tests on all possible permutations of our build targets (including linting, which is listed as a target to prevent linting the same code multiple times).

To add a new target to the build matrix, simply add a new line under ``env``.

More information can be found by reading our [.travis.yml](.travis.yml) file.

## Dependencies

- GNU ARM Embedded toolchain
- GNU Make 4.0 or above
- [Unity&mdash;Throw the Switch](http://www.throwtheswitch.org/unity/): our C unit testing framework
- [ms-common](https://github.com/uw-midsun/ms-common): our Hardware Abstraction Layer, and other shared libraries

### Optional Dependencies

- [Clang/LLVM toolchain](http://releases.llvm.org/download.html)
- [Bear (Build EAR)](https://github.com/rizsotto/Bear)

## Contributions

Before submitting an issue or a pull request to the project, please take a moment to review our code style guide first.

## License

The firmware is made available under the [MIT License](https://opensource.org/licenses/MIT).
