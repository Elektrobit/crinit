# Unit Tests

This directory contains unit tests that can be built from the top directory by
passing `-DUNIT_TESTS=On` to cmake. A normal build with `./ci/build.sh` sets
this option for the aarch64 architecture and will build all unit tests. Each
test produces a separate binary in the build output directory. To run all test
binaries, use the script `./ci/run-utests.sh`.

## File Hierarchy

Each unit test resides in its own directory. The files related to a unit test
follow the naming scheme as shown below.

```
test/
|-- mocks
|   |-- mock-glob-opt-set.c                     The mock for crinitGlobOptSet()
|   |-- mock-glot-opt-set.h                     The corresponding header for the mock
|   `-- mock-*.[ch]                             Additional mocks for functions used in tests
|-- utest-crinit-function-name                  Tests for EBCL_crinitFunctionName()
|   |-- case-glob-opt-error.c                   Testcase for an expected error
|   |-- case-success.c                          Testcase for successful run
|   |-- case-*.c                                More testcases as needed
|   |-- utest-crinit-function-name.c            Main C file for the testsuite
|   `-- utest-crinit-function-name.h            Header with declarations for all testcases
|-- utest-*                                     More tests for other functions
`-- unit_test.h                                 Common header file including the cmocka library
```
