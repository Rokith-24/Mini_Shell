#include "imc.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

void history_add(const imc_config_t *config, const char *command)
{
    FILE *file;
    time_t now;
    struct tm *time_info;
    char timestamp[32];
    if (config == NULL || command == NULL || command[0] == '\0') return;
    file = fopen(config->history_path, "a");
    if (file == NULL) {
        logger_log(LOG_WARNING, "Cannot open history file %s", config->history_path);
        return;
    }
    now = time(NULL);
    time_info = localtime(&now);
    if (time_info != NULL) (void)strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", time_info);
    else (void)snprintf(timestamp, sizeof(timestamp), "unknown-time");
    (void)fprintf(file, "%s,%s\n", timestamp, command);
    (void)fclose(file);
}

void history_show(const imc_config_t *config, size_t limit)
{
    FILE *file;
    char **lines;
    char buffer[MAX_LOG_LINE];
    size_t count = 0U;
    size_t i;
    if (config == NULL) return;
    if (limit == 0U) limit = config->history_size;
    lines = (char **)calloc(limit, sizeof(char *));
    if (lines == NULL) {
        (void)fprintf(stderr, "history: memory allocation failed\n");
        return;
    }
    file = fopen(config->history_path, "r");
    if (file == NULL) {
        (void)printf("No command history yet.\n");
        free(lines);
        return;
    }
    while (fgets(buffer, sizeof(buffer), file) != NULL) {
        char *copy = strdup(buffer);
        if (copy == NULL) break;
        if (count < limit) lines[count++] = copy;
        else {
            free(lines[0]);
            for (i = 1U; i < limit; i++) lines[i - 1U] = lines[i];
            lines[limit - 1U] = copy;
        }
    }
    (void)fclose(file);
    for (i = 0U; i < count; i++) {
        (void)printf("%zu  %s", i + 1U, lines[i]);
        free(lines[i]);
    }
    free(lines);
}
