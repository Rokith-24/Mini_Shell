#include "imc.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <signal.h>
#include <errno.h>

static void child_run(shell_context_t *context, command_stage_t *stage)
{
    bool handled = false;
    int status;
    status = builtin_execute(context, stage, &handled);
    if (handled) _exit(status);
    execvp(stage->argv[0], stage->argv);
    (void)fprintf(stderr, "%s: command not found or cannot execute: %s\n", stage->argv[0], strerror(errno));
    _exit(127);
}

int shell_execute_pipeline(shell_context_t *context, pipeline_t *pipeline)
{
    int pipes[MAX_PIPE_STAGES - 1U][2];
    pid_t pids[MAX_PIPE_STAGES];
    pid_t pgid = 0;
    size_t i;
    size_t forked = 0U;
    int status = 0;
    int wait_status = 0;

    if (context == NULL || pipeline == NULL || pipeline->stage_count == 0U) return 0;

    if (pipeline->stage_count == 1U && !pipeline->background) {
        bool handled = false;
        int builtin_status = builtin_execute(context, &pipeline->stages[0], &handled);
        if (handled) {
            audit_record(&context->config, getpid(), pipeline->display, builtin_status);
            return builtin_status;
        }
    }

    for (i = 0U; i + 1U < pipeline->stage_count; i++) {
        if (pipe(pipes[i]) != 0) {
            (void)perror("pipe");
            while (i > 0U) { i--; (void)close(pipes[i][0]); (void)close(pipes[i][1]); }
            return 1;
        }
    }

    for (i = 0U; i < pipeline->stage_count; i++) {
        pid_t pid = fork();
        if (pid < 0) {
            (void)perror("fork");
            status = 1;
            break;
        }
        if (pid == 0) {
            size_t j;
            pid_t child_group = (pgid == 0) ? getpid() : pgid;
            (void)setpgid(0, child_group);
            (void)signal(SIGINT, SIG_DFL);
            (void)signal(SIGQUIT, SIG_DFL);
            (void)signal(SIGTSTP, SIG_DFL);
            (void)signal(SIGTTIN, SIG_DFL);
            (void)signal(SIGTTOU, SIG_DFL);
            if (pipe_connect_child(pipes, i, pipeline->stage_count) != IMC_OK) _exit(126);
            for (j = 0U; j + 1U < pipeline->stage_count; j++) {
                (void)close(pipes[j][0]);
                (void)close(pipes[j][1]);
            }
            if (redirection_apply(&pipeline->stages[i]) != IMC_OK) _exit(126);
            child_run(context, &pipeline->stages[i]);
        }
        if (pgid == 0) pgid = pid;
        (void)setpgid(pid, pgid);
        forked++;
        pids[i] = pid;
    }

    pipe_close_all(pipes, pipeline->stage_count - 1U);

    if (forked < pipeline->stage_count && pgid > 0) {
        /* A fork failed before all stages were created. */
        (void)kill(-pgid, SIGTERM);
    }

    if (pgid > 0 && pipeline->background) {
        jobs_add(context, pgid, pipeline->display, JOB_RUNNING);
        audit_record(&context->config, pgid, pipeline->display, 0);
        return 0;
    }

    if (pgid > 0) {
        if (context->interactive) (void)tcsetpgrp(STDIN_FILENO, pgid);
        for (i = 0U; i < pipeline->stage_count && i < MAX_PIPE_STAGES; i++) {
            pid_t waited;
            do { waited = waitpid(pids[i], &wait_status, WUNTRACED); }
            while (waited < 0 && errno == EINTR);
            if (waited > 0) {
                if (WIFSTOPPED(wait_status)) {
                    jobs_add(context, pgid, pipeline->display, JOB_STOPPED);
                    break;
                }
                if (i + 1U == pipeline->stage_count) {
                    if (WIFEXITED(wait_status)) status = WEXITSTATUS(wait_status);
                    else if (WIFSIGNALED(wait_status)) status = 128 + WTERMSIG(wait_status);
                }
            }
        }
        if (context->interactive) (void)tcsetpgrp(STDIN_FILENO, context->shell_pgid);
        audit_record(&context->config, pgid, pipeline->display, status);
    }
    return status;
}
