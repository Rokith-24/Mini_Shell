#include "imc.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>

static void ensure_runtime_directories(void)
{
    (void)mkdir("data", 0755);
}

int main(void)
{
    shell_context_t context;
    int result;
    (void)memset(&context, 0, sizeof(context));
    context.next_job_id = 1U;
    ensure_runtime_directories();
    if (config_load(&context.config, "config/imc.conf") != IMC_OK) {
        (void)fprintf(stderr, "Unable to load configuration; using defaults.\n");
    }
    logger_init(context.config.log_path);
    logger_log(LOG_INFO, "IMC started");
    (void)printf("Infrastructure Management Console (IMC)\n");
    (void)printf("Type 'help' for commands. Type 'exit' to quit.\n");
    result = shell_run(&context);
    logger_log(LOG_INFO, "IMC stopped with status %d", result);
    logger_close();
    return result;
}
