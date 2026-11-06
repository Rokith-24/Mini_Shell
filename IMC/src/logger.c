#include "imc.h"
#include <stdio.h>
#include <stdarg.h>
#include <time.h>

static FILE *logger_file = NULL;

void logger_init(const char *path)
{
    if (path != NULL) {
        logger_file = fopen(path, "a");
    }
}

void logger_log(log_level_t level, const char *format, ...)
{
    static const char *names[] = {"DEBUG", "INFO", "WARNING", "ERROR", "FATAL"};
    char timestamp[32];
    time_t now = time(NULL);
    struct tm *time_info = localtime(&now);
    va_list args;

    if (time_info != NULL) {
        (void)strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", time_info);
    } else {
        (void)snprintf(timestamp, sizeof(timestamp), "unknown-time");
    }

    (void)fprintf(stderr, "[%s] %-7s ", timestamp,
                   (level >= LOG_DEBUG && level <= LOG_FATAL) ? names[level] : "UNKNOWN");
    va_start(args, format);
    (void)vfprintf(stderr, format, args);
    va_end(args);
    (void)fputc('\n', stderr);

    if (logger_file != NULL) {
        (void)fprintf(logger_file, "[%s] %-7s ", timestamp,
                      (level >= LOG_DEBUG && level <= LOG_FATAL) ? names[level] : "UNKNOWN");
        va_start(args, format);
        (void)vfprintf(logger_file, format, args);
        va_end(args);
        (void)fputc('\n', logger_file);
        (void)fflush(logger_file);
    }
}

void logger_close(void)
{
    if (logger_file != NULL) {
        (void)fclose(logger_file);
        logger_file = NULL;
    }
}
