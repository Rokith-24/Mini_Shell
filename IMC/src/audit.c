#include "imc.h"
#include <stdio.h>
#include <string.h>
#include <time.h>

void audit_record(const imc_config_t *config, pid_t pid, const char *command, int status)
{
    FILE *file;
    time_t now;
    struct tm *time_info;
    char timestamp[32];
    if (config == NULL || command == NULL) return;
    file = fopen(config->audit_path, "a");
    if (file == NULL) {
        logger_log(LOG_ERROR, "Cannot write audit file %s", config->audit_path);
        return;
    }
    now = time(NULL);
    time_info = localtime(&now);
    if (time_info != NULL) (void)strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", time_info);
    else (void)snprintf(timestamp, sizeof(timestamp), "unknown-time");
    (void)fprintf(file, "%s,%ld,%s,%s\n", timestamp, (long)pid, command,
                  (status == 0) ? "SUCCESS" : "FAILED");
    (void)fclose(file);
}

void audit_report(const imc_config_t *config)
{
    FILE *file;
    char line[MAX_LOG_LINE];
    unsigned long total = 0UL;
    unsigned long success = 0UL;
    unsigned long failed = 0UL;
    if (config == NULL) return;
    file = fopen(config->audit_path, "r");
    if (file == NULL) {
        (void)printf("No audit records found.\n");
        return;
    }
    while (fgets(line, sizeof(line), file) != NULL) {
        total++;
        if (strstr(line, ",SUCCESS") != NULL) success++;
        else failed++;
        (void)fputs(line, stdout);
    }
    (void)fclose(file);
    (void)printf("\nAudit summary: total=%lu success=%lu failed=%lu\n", total, success, failed);
}
