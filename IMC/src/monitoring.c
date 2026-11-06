#include "imc.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/statvfs.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/types.h>

static void print_proc_summary(void)
{
    FILE *file;
    char line[256];
    file = fopen("/proc/meminfo", "r");
    if (file != NULL) {
        (void)printf("Memory:\n");
        if (fgets(line, sizeof(line), file) != NULL) (void)printf("  %s", line);
        if (fgets(line, sizeof(line), file) != NULL) (void)printf("  %s", line);
        (void)fclose(file);
    } else {
        (void)printf("Memory information unavailable (/proc/meminfo).\n");
    }
    file = fopen("/proc/uptime", "r");
    if (file != NULL) {
        double uptime = 0.0;
        if (fscanf(file, "%lf", &uptime) == 1) {
            (void)printf("Uptime: %.0f seconds\n", uptime);
        }
        (void)fclose(file);
    }
}


static double process_average_cpu(long pid)
{
    char path[MAX_PATH_LENGTH];
    char stat_line[2048];
    char *right_parenthesis;
    char *save_pointer = NULL;
    char *field;
    unsigned int field_number = 3U;
    unsigned long long user_ticks = 0ULL;
    unsigned long long system_ticks = 0ULL;
    unsigned long long start_ticks = 0ULL;
    double uptime = 0.0;
    long ticks_per_second = sysconf(_SC_CLK_TCK);
    FILE *file;

    if (ticks_per_second <= 0L) return 0.0;
    if (snprintf(path, sizeof(path), "/proc/%ld/stat", pid) < 0) return 0.0;
    file = fopen(path, "r");
    if (file == NULL) return 0.0;
    if (fgets(stat_line, sizeof(stat_line), file) == NULL) {
        (void)fclose(file);
        return 0.0;
    }
    (void)fclose(file);

    right_parenthesis = strrchr(stat_line, ')');
    if (right_parenthesis == NULL) return 0.0;
    field = strtok_r(right_parenthesis + 2, " ", &save_pointer);
    while (field != NULL) {
        char *end = NULL;
        unsigned long long value = strtoull(field, &end, 10);
        if (field_number == 14U) user_ticks = value;
        else if (field_number == 15U) system_ticks = value;
        else if (field_number == 22U) {
            start_ticks = value;
            break;
        }
        field_number++;
        field = strtok_r(NULL, " ", &save_pointer);
    }

    file = fopen("/proc/uptime", "r");
    if (file == NULL) return 0.0;
    if (fscanf(file, "%lf", &uptime) != 1) {
        (void)fclose(file);
        return 0.0;
    }
    (void)fclose(file);

    {
        double elapsed = uptime - ((double)start_ticks / (double)ticks_per_second);
        double cpu_seconds = (double)(user_ticks + system_ticks) / (double)ticks_per_second;
        if (elapsed <= 0.0) return 0.0;
        return (cpu_seconds / elapsed) * 100.0;
    }
}

static int log_search(const char *path, const char *keyword)
{
    FILE *file = fopen(path, "r");
    char line[MAX_LOG_LINE];
    unsigned long matches = 0UL;
    if (file == NULL) {
        (void)fprintf(stderr, "log-search: cannot open %s\n", path);
        return 1;
    }
    while (fgets(line, sizeof(line), file) != NULL) {
        if (strstr(line, keyword) != NULL) {
            (void)fputs(line, stdout);
            matches++;
        }
    }
    (void)fclose(file);
    (void)printf("Matches: %lu\n", matches);
    return 0;
}

int monitoring_execute(shell_context_t *context, command_stage_t *stage, bool *handled)
{
    const char *name;
    *handled = false;
    if (stage == NULL || stage->argc == 0U) return 0;
    name = stage->argv[0];

    if (strcmp(name, "healthcheck") == 0 || strcmp(name, "system-summary") == 0) {
        struct statvfs disk;
        *handled = true;
        (void)printf("IMC Infrastructure Health Check\n");
        {
            char hostname[256];
            if (gethostname(hostname, sizeof(hostname)) == 0) {
                hostname[sizeof(hostname) - 1U] = '\0';
                (void)printf("Hostname: %s\n", hostname);
            }
        }
        print_proc_summary();
        if (statvfs("/", &disk) == 0) {
            unsigned long long free_bytes = (unsigned long long)disk.f_bavail *
                                            (unsigned long long)disk.f_frsize;
            (void)printf("Disk available: %llu bytes\n", free_bytes);
        }
        {
            char cwd[MAX_PATH_LENGTH];
            if (getcwd(cwd, sizeof(cwd)) != NULL) {
                (void)printf("Current directory: %s\n", cwd);
            }
        }
        return 0;
    }
    if (strcmp(name, "service-status") == 0) {
        DIR *proc;
        struct dirent *entry;
        const char *filter = (stage->argc > 1U) ? stage->argv[1] : NULL;
        *handled = true;
        (void)printf("%-8s %-24s %-10s %s\n", "PID", "PROCESS", "CPU(avg)", "MEMORY");
        proc = opendir("/proc");
        if (proc == NULL) {
            (void)perror("service-status: /proc");
            return 1;
        }
        while ((entry = readdir(proc)) != NULL) {
            char *end = NULL;
            long pid = strtol(entry->d_name, &end, 10);
            if (end != entry->d_name && *end == '\0' && pid > 0L) {
                char path[MAX_PATH_LENGTH];
                char comm[256] = "unknown";
                char memory[64] = "-";
                FILE *process_file;
                if (snprintf(path, sizeof(path), "/proc/%ld/comm", pid) < 0) continue;
                process_file = fopen(path, "r");
                if (process_file != NULL) {
                    if (fgets(comm, sizeof(comm), process_file) != NULL) {
                        comm[strcspn(comm, "\r\n")] = '\0';
                    }
                    (void)fclose(process_file);
                }
                if (filter != NULL && strstr(comm, filter) == NULL) continue;
                if (snprintf(path, sizeof(path), "/proc/%ld/status", pid) >= 0) {
                    process_file = fopen(path, "r");
                    if (process_file != NULL) {
                        char status_line[256];
                        while (fgets(status_line, sizeof(status_line), process_file) != NULL) {
                            if (strncmp(status_line, "VmRSS:", 6U) == 0) {
                                (void)snprintf(memory, sizeof(memory), "%.*s", (int)(sizeof(memory) - 1U), status_line + 6U);
                                memory[strcspn(memory, "\r\n")] = '\0';
                                break;
                            }
                        }
                        (void)fclose(process_file);
                    }
                }
                (void)printf("%-8ld %-24.24s %-9.2f%% %s\n", pid, comm, process_average_cpu(pid), memory);
            }
        }
        (void)closedir(proc);
        return 0;
    }
    if (strcmp(name, "log-search") == 0) {
        *handled = true;
        if (stage->argc < 2U) {
            (void)fprintf(stderr, "Usage: log-search KEYWORD [FILE]\n");
            return 2;
        }
        return log_search((stage->argc > 2U) ? stage->argv[2] : "data/imc.log", stage->argv[1]);
    }
    if (strcmp(name, "log-summary") == 0) {
        FILE *file;
        char line[MAX_LOG_LINE];
        unsigned long lines = 0UL;
        *handled = true;
        file = fopen((stage->argc > 1U) ? stage->argv[1] : "data/imc.log", "r");
        if (file == NULL) {
            (void)printf("No log file found.\n");
            return 1;
        }
        while (fgets(line, sizeof(line), file) != NULL) lines++;
        (void)fclose(file);
        (void)printf("Log lines: %lu\n", lines);
        return 0;
    }
    if (strcmp(name, "cleanup") == 0) {
        *handled = true;
        (void)printf("Cleanup: removing generated report files and temporary files only.\n");
        (void)system("find reports -type f -name '*.tmp' -delete 2>/dev/null");
        return 0;
    }
    if (strcmp(name, "audit-report") == 0) {
        *handled = true;
        audit_report(&context->config);
        return 0;
    }
    return 0;
}
