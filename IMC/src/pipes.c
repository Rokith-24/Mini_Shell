#include "imc.h"
#include <unistd.h>
#include <stdio.h>

int pipe_connect_child(int pipes[][2], size_t stage_index, size_t stage_count)
{
    if (pipes == NULL || stage_count == 0U) return IMC_ERROR;

    if (stage_index > 0U &&
        dup2(pipes[stage_index - 1U][0], STDIN_FILENO) < 0) {
        (void)perror("dup2 pipe input");
        return IMC_ERROR;
    }
    if (stage_index + 1U < stage_count &&
        dup2(pipes[stage_index][1], STDOUT_FILENO) < 0) {
        (void)perror("dup2 pipe output");
        return IMC_ERROR;
    }
    return IMC_OK;
}

void pipe_close_all(int pipes[][2], size_t pipe_count)
{
    size_t index;
    if (pipes == NULL) return;
    for (index = 0U; index < pipe_count; index++) {
        (void)close(pipes[index][0]);
        (void)close(pipes[index][1]);
    }
}
