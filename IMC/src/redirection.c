#include "imc.h"
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>

int redirection_apply(const command_stage_t *stage)
{
    int descriptor;
    if (stage == NULL) return IMC_ERROR;

    if (stage->input_file[0] != '\0') {
        descriptor = open(stage->input_file, O_RDONLY);
        if (descriptor < 0) {
            (void)perror(stage->input_file);
            return IMC_ERROR;
        }
        if (dup2(descriptor, STDIN_FILENO) < 0) {
            (void)perror("dup2 input");
            (void)close(descriptor);
            return IMC_ERROR;
        }
        (void)close(descriptor);
    }

    if (stage->output_file[0] != '\0') {
        int flags = O_WRONLY | O_CREAT |
                    (stage->append_output ? O_APPEND : O_TRUNC);
        descriptor = open(stage->output_file, flags, 0644);
        if (descriptor < 0) {
            (void)perror(stage->output_file);
            return IMC_ERROR;
        }
        if (dup2(descriptor, STDOUT_FILENO) < 0) {
            (void)perror("dup2 output");
            (void)close(descriptor);
            return IMC_ERROR;
        }
        (void)close(descriptor);
    }
    return IMC_OK;
}
