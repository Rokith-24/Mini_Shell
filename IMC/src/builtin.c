#include "imc.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>
#include <sys/wait.h>

bool builtin_is_parent_only(const char *name)
{
    return (name != NULL && (strcmp(name, "cd") == 0 || strcmp(name, "exit") == 0 ||
            strcmp(name, "fg") == 0 || strcmp(name, "bg") == 0 || strcmp(name, "kill-job") == 0));
}

int builtin_execute(shell_context_t *context, command_stage_t *stage, bool *handled)
{
    const char *name;
    int result = 0;
    *handled = false;
    if (context == NULL || stage == NULL || stage->argc == 0U) return 0;
    name = stage->argv[0];

    if (strcmp(name, "cd") == 0) {
        const char *target = (stage->argc > 1U) ? stage->argv[1] : getenv("HOME");
        *handled = true;
        if (target == NULL || chdir(target) != 0) {
            (void)perror("cd");
            return 1;
        }
    } else if (strcmp(name, "pwd") == 0) {
        char cwd[MAX_PATH_LENGTH];
        *handled = true;
        if (getcwd(cwd, sizeof(cwd)) == NULL) { (void)perror("pwd"); return 1; }
        (void)printf("%s\n", cwd);
    } else if (strcmp(name, "help") == 0) {
        *handled = true;
        (void)printf("IMC built-ins:\n");
        (void)printf("  cd [DIR]  pwd  exit  help  history  jobs  fg JOB  bg JOB  kill-job JOB\n");
        (void)printf("  service-status [NAME]  log-search KEYWORD [FILE]  log-summary [FILE]\n");
        (void)printf("  healthcheck  system-summary  cleanup  audit-report\n");
        (void)printf("Shell syntax: command | command, < input, > output, >> output, command &, command ; command\n");
    } else if (strcmp(name, "exit") == 0 || strcmp(name, "quit") == 0) {
        *handled = true;
        context->should_exit = true;
        if (stage->argc > 1U) context->last_status = atoi(stage->argv[1]);
    } else if (strcmp(name, "history") == 0) {
        *handled = true;
        history_show(&context->config, (stage->argc > 1U) ? (size_t)strtoul(stage->argv[1], NULL, 10) : 0U);
    } else if (strcmp(name, "jobs") == 0) {
        *handled = true;
        jobs_reap(context);
        jobs_list(context);
    } else if (strcmp(name, "fg") == 0 || strcmp(name, "bg") == 0 || strcmp(name, "kill-job") == 0) {
        job_t *job;
        uint32_t id;
        *handled = true;
        if (stage->argc < 2U) { (void)fprintf(stderr, "%s: job id required\n", name); return 2; }
        id = (uint32_t)strtoul(stage->argv[1], NULL, 10);
        job = jobs_find(context, id);
        if (job == NULL) { (void)fprintf(stderr, "%s: no such job: %u\n", name, id); return 1; }
        if (strcmp(name, "kill-job") == 0) {
            if (kill(-job->pgid, SIGTERM) != 0) { (void)perror("kill-job"); return 1; }
            return 0;
        }
        if (strcmp(name, "bg") == 0) {
            if (kill(-job->pgid, SIGCONT) != 0) { (void)perror("bg"); return 1; }
            job->state = JOB_RUNNING;
            (void)printf("[%u] %s &\n", job->job_id, job->command);
            return 0;
        }
        if (kill(-job->pgid, SIGCONT) != 0 && errno != ESRCH) { (void)perror("fg"); return 1; }
        if (context->interactive) (void)tcsetpgrp(STDIN_FILENO, job->pgid);
        {
            int status = 0;
            pid_t waited;
            do { waited = waitpid(-job->pgid, &status, WUNTRACED); }
            while (waited < 0 && errno == EINTR);
            if (context->interactive) (void)tcsetpgrp(STDIN_FILENO, context->shell_pgid);
            if (waited > 0 && WIFSTOPPED(status)) job->state = JOB_STOPPED;
            else jobs_remove(context, id);
            if (waited > 0 && WIFEXITED(status)) result = WEXITSTATUS(status);
        }
    } else {
        bool monitoring_handled = false;
        result = monitoring_execute(context, stage, &monitoring_handled);
        *handled = monitoring_handled;
    }
    return result;
}
