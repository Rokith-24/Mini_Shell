#include "imc.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>

static volatile sig_atomic_t child_changed = 0;

static void child_signal_handler(int signal_number)
{
    (void)signal_number;
    child_changed = 1;
}

static void setup_signals(shell_context_t *context)
{
    struct sigaction action;
    (void)memset(&action, 0, sizeof(action));
    action.sa_handler = child_signal_handler;
    sigemptyset(&action.sa_mask);
    action.sa_flags = SA_RESTART;
    (void)sigaction(SIGCHLD, &action, NULL);

    (void)signal(SIGINT, SIG_IGN);
    (void)signal(SIGQUIT, SIG_IGN);
    (void)signal(SIGTSTP, SIG_IGN);
    (void)signal(SIGTTIN, SIG_IGN);
    (void)signal(SIGTTOU, SIG_IGN);

    if (context->interactive) {
        context->shell_pgid = getpid();
        (void)setpgid(context->shell_pgid, context->shell_pgid);
        (void)tcsetpgrp(STDIN_FILENO, context->shell_pgid);
    }
}

static void free_parsed(parsed_line_t *parsed)
{
    size_t i;
    size_t j;
    size_t k;
    for (i = 0U; i < parsed->command_count; i++) {
        for (j = 0U; j < parsed->commands[i].stage_count; j++) {
            command_stage_t *stage = &parsed->commands[i].stages[j];
            for (k = 0U; k < stage->argc; k++) {
                free(stage->argv[k]);
                stage->argv[k] = NULL;
            }
        }
    }
}

int shell_run(shell_context_t *context)
{
    char input[MAX_INPUT_LENGTH];
    char error[256];

    if (context == NULL) return IMC_ERROR;
    context->interactive = isatty(STDIN_FILENO);
    setup_signals(context);

    while (!context->should_exit) {
        parsed_line_t parsed;
        size_t i;
        if (child_changed != 0) {
            jobs_reap(context);
            child_changed = 0;
        }
        if (context->interactive) {
            (void)printf("IMC> ");
            (void)fflush(stdout);
        }
        if (fgets(input, sizeof(input), stdin) == NULL) {
            if (feof(stdin) != 0) break;
            if (errno == EINTR) { clearerr(stdin); continue; }
            (void)perror("fgets");
            break;
        }
        if (strchr(input, '\n') == NULL && !feof(stdin)) {
            int character;
            while ((character = getchar()) != '\n' && character != EOF) { }
            (void)fprintf(stderr, "Input too long (maximum %u characters).\n", MAX_INPUT_LENGTH - 1U);
            context->last_status = 2;
            continue;
        }
        input[strcspn(input, "\r\n")] = '\0';
        if (input[0] == '\0') continue;

        history_add(&context->config, input);
        if (parse_line(input, &parsed, error, sizeof(error)) != IMC_OK) {
            (void)fprintf(stderr, "parse error: %s\n", error);
            logger_log(LOG_WARNING, "Parse error: %s", error);
            context->last_status = 2;
            continue;
        }
        for (i = 0U; i < parsed.command_count && !context->should_exit; i++) {
            context->last_status = shell_execute_pipeline(context, &parsed.commands[i]);
        }
        free_parsed(&parsed);
    }
    jobs_cleanup(context);
    return context->last_status;
}
