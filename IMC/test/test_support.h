#ifndef IMC_TEST_SUPPORT_H
#define IMC_TEST_SUPPORT_H
#include <gtest/gtest.h>
extern "C" {
#include "imc.h"
}
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iterator>
#include <string>
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>
static std::string temp_name(const char *suffix) {
    return std::string("/tmp/imc_gtest_") + std::to_string((long)getpid()) + "_" + suffix;
}
static void free_parsed(parsed_line_t *p) {
    if (!p) return;
    for (size_t i=0;i<p->command_count;i++)
        for (size_t j=0;j<p->commands[i].stage_count;j++)
            for (size_t k=0;k<p->commands[i].stages[j].argc;k++)
                std::free(p->commands[i].stages[j].argv[k]);
}
#endif
