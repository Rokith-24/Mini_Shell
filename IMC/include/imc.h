#ifndef IMC_H
#define IMC_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <sys/types.h>

#define MAX_INPUT_LENGTH        1024U
#define MAX_TOKEN_LENGTH        256U
#define MAX_ARGS                64U
#define MAX_PIPE_STAGES         8U
#define MAX_COMMANDS            8U
#define MAX_JOBS                128U
#define MAX_PATH_LENGTH         512U
#define MAX_LOG_LINE            1024U
#define DEFAULT_HISTORY_SIZE    500U

typedef enum {
    IMC_OK = 0,
    IMC_ERROR = -1,
    IMC_PARSE_ERROR = -2,
    IMC_LIMIT_ERROR = -3
} imc_status_t;

typedef enum {
    LOG_DEBUG = 0,
    LOG_INFO,
    LOG_WARNING,
    LOG_ERROR,
    LOG_FATAL
} log_level_t;

typedef enum {
    JOB_RUNNING = 0,
    JOB_STOPPED,
    JOB_DONE
} job_state_t;

typedef struct {
    char *argv[MAX_ARGS + 1U];
    size_t argc;
    char input_file[MAX_PATH_LENGTH];
    char output_file[MAX_PATH_LENGTH];
    bool append_output;
} command_stage_t;

typedef struct {
    command_stage_t stages[MAX_PIPE_STAGES];
    size_t stage_count;
    bool background;
    char display[MAX_INPUT_LENGTH];
} pipeline_t;

typedef struct {
    pipeline_t commands[MAX_COMMANDS];
    size_t command_count;
} parsed_line_t;

typedef struct job {
    uint32_t job_id;
    pid_t pgid;
    char command[MAX_INPUT_LENGTH];
    job_state_t state;
    struct job *next;
} job_t;

typedef struct {
    char history_path[MAX_PATH_LENGTH];
    char audit_path[MAX_PATH_LENGTH];
    char log_path[MAX_PATH_LENGTH];
    size_t history_size;
    bool show_timestamp;
} imc_config_t;

typedef struct {
    bool interactive;
    bool should_exit;
    int last_status;
    pid_t shell_pgid;
    job_t *jobs;
    uint32_t next_job_id;
    imc_config_t config;
} shell_context_t;

void logger_init(const char *path);
void logger_log(log_level_t level, const char *format, ...);
void logger_close(void);

int config_load(imc_config_t *config, const char *path);
void config_defaults(imc_config_t *config);

int parse_line(char *input, parsed_line_t *parsed, char *error, size_t error_size);
int shell_run(shell_context_t *context);
int shell_execute_pipeline(shell_context_t *context, pipeline_t *pipeline);
int redirection_apply(const command_stage_t *stage);
int pipe_connect_child(int pipes[][2], size_t stage_index, size_t stage_count);
void pipe_close_all(int pipes[][2], size_t pipe_count);

void jobs_add(shell_context_t *context, pid_t pgid, const char *command, job_state_t state);
void jobs_list(shell_context_t *context);
job_t *jobs_find(shell_context_t *context, uint32_t job_id);
void jobs_remove(shell_context_t *context, uint32_t job_id);
void jobs_reap(shell_context_t *context);
void jobs_cleanup(shell_context_t *context);

void history_add(const imc_config_t *config, const char *command);
void history_show(const imc_config_t *config, size_t limit);
void audit_record(const imc_config_t *config, pid_t pid, const char *command, int status);
void audit_report(const imc_config_t *config);

int builtin_execute(shell_context_t *context, command_stage_t *stage, bool *handled);
bool builtin_is_parent_only(const char *name);
int monitoring_execute(shell_context_t *context, command_stage_t *stage, bool *handled);

#ifdef __cplusplus
}
#endif

#endif
