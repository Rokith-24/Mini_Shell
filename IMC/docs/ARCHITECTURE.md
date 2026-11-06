# Architecture

The application is a Linux user-space shell written in C11. `src/` is intentionally flat: each implementation module is a single C file, with shared public declarations in `include/imc.h`.

## Modules

- `main.c`: initialization, configuration and shutdown.
- `shell.c`: interactive/non-interactive loop and signal setup.
- `parser.c`: tokenization, quoting, environment expansion, pipelines and operators.
- `process.c`: process creation, execution, waiting and foreground/background dispatch.
- `pipes.c`: pipe descriptor connection and cleanup.
- `redirection.c`: input/output descriptor redirection.
- `jobs.c`: linked-list job table and job state operations.
- `builtin.c`: shell built-ins and parent-only command handling.
- `history.c`: persistent command history.
- `audit.c`: command audit records and summary.
- `logger.c`: runtime diagnostics.
- `monitoring.c`: process/system and log inspection commands.
- `config.c`: runtime configuration loading and defaults.

## Runtime files

All persistent runtime files are stored under `data/`: `history.txt`, `audit.log`, and `imc.log`. Configuration is read from `config/imc.conf`.

## Process model

The shell parses a line into one or more command pipelines. External commands are started using POSIX process APIs (`fork`, `execvp`, `waitpid`). Pipelines use `pipe`/`dup2`; background jobs are tracked by process group. The Linux kernel remains responsible for actual process scheduling and execution.
