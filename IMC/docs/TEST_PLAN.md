# Test Plan

## Framework and language boundary

Google Test is a C++ test framework. Production code remains C11. The Makefile compiles each `src/*.c` file with GCC, compiles each `test/test_*.cpp` suite with G++, and links the C objects to Google Test.

## Suites

- `test_config.cpp`: defaults, missing configuration, valid values, invalid values and null arguments.
- `test_parser.cpp`: words, quotes, environment variables, pipelines, redirection, append, background, command separators and malformed input.
- `test_jobs.cpp`: add/find/remove/list/reap/cleanup and null/empty handling.
- `test_history.cpp`: persistence, display and invalid inputs.
- `test_audit.cpp`: success/failure records, report summary and missing file handling.
- `test_logger.cpp`: log output, levels, null path and close.
- `test_builtin.cpp`: parent-only classification, help, exit and unknown commands.
- `test_monitoring.cpp`: healthcheck, log search/summary, unknown command and invalid arguments.
- `test_pipes.cpp`: pipe connection and invalid arguments.
- `test_redirection.cpp`: output redirection and missing input.
- `test_process.cpp`: external command, pipeline and background execution.
- `test_shell.cpp`: null context and exit input.
- `test_integration.cpp`: executable startup and clean exit.

## Commands

Run `make test` for the complete Google Test suite, `make cppcheck` for static analysis, and `make coverage` for gcovr HTML/XML output.

The coverage target enforces at least 90% line and function coverage. The threshold is a gate, not a pre-reported result: the actual percentage must be obtained by running the tests and coverage tools in the target Linux environment. Any uncovered paths should be addressed with additional focused tests rather than suppressing the threshold.
