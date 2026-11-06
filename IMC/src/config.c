#include "imc.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

static void trim(char *text)
{
    size_t length;
    size_t start = 0U;

    if (text == NULL) return;
    while (isspace((unsigned char)text[start]) != 0) start++;
    if (start > 0U) (void)memmove(text, text + start, strlen(text + start) + 1U);
    length = strlen(text);
    while (length > 0U && isspace((unsigned char)text[length - 1U]) != 0) {
        text[length - 1U] = '\0';
        length--;
    }
}

void config_defaults(imc_config_t *config)
{
    if (config == NULL) return;
    (void)snprintf(config->history_path, sizeof(config->history_path), "data/history.txt");
    (void)snprintf(config->audit_path, sizeof(config->audit_path), "data/audit.log");
    (void)snprintf(config->log_path, sizeof(config->log_path), "data/imc.log");
    config->history_size = DEFAULT_HISTORY_SIZE;
    config->show_timestamp = false;
}

int config_load(imc_config_t *config, const char *path)
{
    FILE *file;
    char line[512];

    if (config == NULL || path == NULL) return IMC_ERROR;
    config_defaults(config);
    file = fopen(path, "r");
    if (file == NULL) return IMC_OK; /* Defaults are valid when no file exists. */

    while (fgets(line, sizeof(line), file) != NULL) {
        char *separator;
        char *key;
        char *value;
        trim(line);
        if (line[0] == '\0' || line[0] == '#') continue;
        separator = strchr(line, '=');
        if (separator == NULL) continue;
        *separator = '\0';
        key = line;
        value = separator + 1;
        trim(key);
        trim(value);

        if (strcmp(key, "history_path") == 0) {
            (void)snprintf(config->history_path, sizeof(config->history_path), "%s", value);
        } else if (strcmp(key, "audit_path") == 0) {
            (void)snprintf(config->audit_path, sizeof(config->audit_path), "%s", value);
        } else if (strcmp(key, "log_path") == 0) {
            (void)snprintf(config->log_path, sizeof(config->log_path), "%s", value);
        } else if (strcmp(key, "history_size") == 0) {
            char *end = NULL;
            unsigned long parsed = strtoul(value, &end, 10);
            if (end != value && *end == '\0' && parsed > 0UL && parsed <= 100000UL) {
                config->history_size = (size_t)parsed;
            }
        } else if (strcmp(key, "show_timestamp") == 0) {
            config->show_timestamp = (strcmp(value, "true") == 0 || strcmp(value, "1") == 0);
        }
    }
    (void)fclose(file);
    return IMC_OK;
}
