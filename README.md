#firmware

[![Build Status](https://travis-ci.org/uw-midsun/firmware.svg?branch=master)](https://travis-ci.org/uw-midsun/firmware)

This repository contains all the latest firmware for the [University of Waterloo](https://uwaterloo.ca/)'s [Midnight Sun Solar Rayce Car](http://www.uwmidsun.com/) team.


## Building and Testing

```bash
git clone https://github.com/uw-midsun/firmware.git firmware
cd firmware
git submodule update --init --recursive
```

We use the GNU ARM Embedded toolchain to build all our firmware.

```bash
make build_all PLATFORM=x86
```

To build a project, run ``make project PROJECT=$PROJECT PLATFORM=$PLATFORM``, where ``$PROJECT`` is a valid project name, and ``$PLATFORM`` is a supported platform.

**Note**: If not specified, ``$PLATFORM`` is set to ``stm32f0xx`` by default.

To program the STM microcontrollers, run ``make program``.

To debug code, run ``make gdb``.

To test a project, run ``make test PROJECT=$PROJECT``, where ``$PROJECT`` is a valid project name.

To test a library, run ``make test LIBRARY=$LIBRARY``, where ``$LIBRARY`` is a valid library name.

To test against code standards, run ``make lint``.

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

* GNU ARM Embedded toolchain
* GNU Make 4.0 or above
* [Unity&mdash;Throw the Switch](http://www.throwtheswitch.org/unity/): our C unit testing framework
* [ms-common](https://github.com/uw-midsun/ms-common): our Hardware Abstraction Layer, and other shared libraries

### Optional Dependencies:

* [Clang/LLVM toolchain](http://releases.llvm.org/download.html)
* [Bear (Build EAR)](https://github.com/rizsotto/Bear)

## Contributions
Before submitting an issue or a pull request to the project, please take a moment to review our code style guide first.

## License
The firmware is made available under the [MIT License](https://opensource.org/licenses/MIT).
