#include "imc.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <errno.h>

void jobs_add(shell_context_t *context, pid_t pgid, const char *command, job_state_t state)
{
    job_t *job;
    if (context == NULL || pgid <= 0) return;
    job = (job_t *)calloc(1U, sizeof(*job));
    if (job == NULL) {
        logger_log(LOG_ERROR, "Unable to allocate job record");
        return;
    }
    job->job_id = context->next_job_id++;
    job->pgid = pgid;
    job->state = state;
    (void)snprintf(job->command, sizeof(job->command), "%s", command);
    job->next = context->jobs;
    context->jobs = job;
    (void)printf("[%u] %ld\n", job->job_id, (long)pgid);
}

job_t *jobs_find(shell_context_t *context, uint32_t job_id)
{
    job_t *current = (context != NULL) ? context->jobs : NULL;
    while (current != NULL) {
        if (current->job_id == job_id) return current;
        current = current->next;
    }
    return NULL;
}

void jobs_remove(shell_context_t *context, uint32_t job_id)
{
    job_t *current;
    job_t *previous = NULL;
    if (context == NULL) return;
    current = context->jobs;
    while (current != NULL) {
        if (current->job_id == job_id) {
            if (previous == NULL) context->jobs = current->next;
            else previous->next = current->next;
            free(current);
            return;
        }
        previous = current;
        current = current->next;
    }
}

void jobs_list(shell_context_t *context)
{
    job_t *current = (context != NULL) ? context->jobs : NULL;
    while (current != NULL) {
        const char *state = (current->state == JOB_RUNNING) ? "Running" :
                            (current->state == JOB_STOPPED) ? "Stopped" : "Done";
        (void)printf("[%u] %-8s %s\n", current->job_id, state, current->command);
        current = current->next;
    }
}

void jobs_reap(shell_context_t *context)
{
    int status = 0;
    pid_t pid;
    job_t *current;
    if (context == NULL) return;
    while ((pid = waitpid(-1, &status, WNOHANG | WUNTRACED | WCONTINUED)) > 0) {
        current = context->jobs;
        while (current != NULL) {
            if (current->pgid == pid) {
                if (WIFEXITED(status) || WIFSIGNALED(status)) current->state = JOB_DONE;
                else if (WIFSTOPPED(status)) current->state = JOB_STOPPED;
                else if (WIFCONTINUED(status)) current->state = JOB_RUNNING;
                break;
            }
            current = current->next;
        }
    }
    if (pid < 0 && errno != ECHILD && errno != EINTR) {
        logger_log(LOG_WARNING, "waitpid while reaping jobs: %s", strerror(errno));
    }
}

void jobs_cleanup(shell_context_t *context)
{
    job_t *current;
    if (context == NULL) return;
    current = context->jobs;
    while (current != NULL) {
        job_t *next = current->next;
        free(current);
        current = next;
    }
    context->jobs = NULL;
}
