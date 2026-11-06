# Infrastructure Management Console (IMC)

A Bash-like Linux command-line interpreter and Infrastructure Management Console implemented in C11.

## Platform and dependencies

- Linux, including RHEL-compatible distributions
- GCC and G++
- GNU Make
- Google Test (for the C++ test harness; application code remains C)
- Cppcheck (static analysis)
- gcovr (coverage report and enforced thresholds)
- Optional: Valgrind

Install the development tools using the package manager approved for your Linux environment. On RHEL-like systems, package names may vary by enabled repositories.

## Build and run

```bash
make
make run
```

Or launch directly:

```bash
./build/imc
```

Type `help` inside the shell to list available built-ins. Type `exit` to quit.

## Supported shell features

### Built-in commands

```text
help
pwd
cd [directory]
history [count]
jobs
fg JOB_ID
bg JOB_ID
kill-job JOB_ID
exit [status]
```

### Command execution and operators

```bash
ls -l
sleep 30 &
ls | wc -l
cat < input.txt
ls > output.txt
echo hello >> output.txt
pwd ; date
echo "$HOME"
```

### Infrastructure and log commands

```text
service-status [process-name]
log-search KEYWORD [FILE]
log-summary [FILE]
healthcheck
system-summary
cleanup
audit-report
```

`service-status` reports process-oriented information. Use `systemctl status SERVICE` as an external command for systemd unit status.

## Quality checks

```bash
make test       # Google Test suites
make cppcheck   # C and C++ static analysis
make coverage   # gcovr report; enforces >90% line and function coverage
make valgrind   # optional memory analysis
make helgrind   # optional thread analysis
make clean
```

Coverage output is generated under `build/coverage/`. The coverage target returns a failure if either line or function coverage is below 90%; it does not claim a percentage before the tests are run on the target environment.

## Configuration and persistent data

Edit `config/imc.conf`:

```ini
history_path=data/history.txt
audit_path=data/audit.log
log_path=data/imc.log
history_size=500
show_timestamp=false
```

The `data/` directory contains exactly the three persistent runtime files:

- `history.txt` — command history
- `audit.log` — command audit records
- `imc.log` — application diagnostics

## Project directory structure

```text
IMC/
├── src/                 All C implementation files (flat)
│   ├── main.c
│   ├── shell.c
│   ├── parser.c
│   ├── process.c
│   ├── jobs.c
│   ├── pipes.c
│   ├── redirection.c
│   ├── history.c
│   ├── audit.c
│   ├── logger.c
│   ├── builtin.c
│   ├── monitoring.c
│   └── config.c
├── include/
│   └── imc.h
├── test/                Separate Google Test C++ test file per module
│   ├── test_support.h
│   └── test_*.cpp
├── data/
│   ├── history.txt
│   ├── audit.log
│   └── imc.log
├── config/
│   └── imc.conf
├── docs/
│   ├── ARCHITECTURE.md
│   └── TEST_PLAN.md
├── build/               Executables, object files and coverage output
├── Makefile
└── README.md
```

## Test design

Each functional module has its own `test_<module>.cpp` Google Test suite. The C source files are compiled as C objects and linked to the C++ Google Test runner. The production application remains C; C++ is used only because Google Test is a C++ testing framework.

## Engineering notes and limitations

- This is a learning/project shell, not a security-hardened replacement for Bash.
- The parser supports bounded arguments, quoting, environment expansion, pipelines, redirection, background execution and semicolon-separated commands. It does not implement globbing, command substitution, here-documents, `&&`, `||`, or full shell grammar.
- Job control is a practical baseline; complicated terminal-control edge cases and multi-process stopped pipelines may need further hardening.
- Access to protected system logs may require suitable permissions.
- MISRA C compliance is not claimed. POSIX APIs and dynamic allocation require project-specific review and documented deviations for a formal compliance claim.

## Safety

IMC executes external commands with the privileges of the current user. Avoid running destructive commands as root.
