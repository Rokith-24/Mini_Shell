#include "imc.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

typedef enum { TOK_WORD = 0, TOK_PIPE, TOK_IN, TOK_OUT, TOK_APPEND, TOK_AMP, TOK_SEMI } token_type_t;
typedef struct { token_type_t type; char text[MAX_TOKEN_LENGTH]; } token_t;

static int add_token(token_t *tokens, size_t *count, token_type_t type, const char *text)
{
    if (*count >= (MAX_ARGS * MAX_PIPE_STAGES * MAX_COMMANDS)) return IMC_LIMIT_ERROR;
    tokens[*count].type = type;
    if (text != NULL) {
        (void)snprintf(tokens[*count].text, sizeof(tokens[*count].text), "%s", text);
    } else {
        tokens[*count].text[0] = '\0';
    }
    (*count)++;
    return IMC_OK;
}

static int expand_variable(const char *input, size_t *index, char *output, size_t *out_len)
{
    char name[128];
    size_t name_len = 0U;
    const char *value;
    size_t i = *index + 1U;

    if (input[i] == '{') {
        i++;
        while (input[i] != '\0' && input[i] != '}' && name_len < sizeof(name) - 1U) {
            name[name_len++] = input[i++];
        }
        if (input[i] != '}') return IMC_PARSE_ERROR;
        *index = i;
    } else {
        while ((isalnum((unsigned char)input[i]) != 0 || input[i] == '_') &&
               name_len < sizeof(name) - 1U) {
            name[name_len++] = input[i++];
        }
        *index = i - 1U;
        if (name_len == 0U) {
            if (*out_len + 1U >= MAX_TOKEN_LENGTH) return IMC_LIMIT_ERROR;
            output[(*out_len)++] = '$';
            return IMC_OK;
        }
    }
    name[name_len] = '\0';
    value = getenv(name);
    if (value != NULL) {
        size_t value_len = strlen(value);
        if (*out_len + value_len >= MAX_TOKEN_LENGTH) return IMC_LIMIT_ERROR;
        (void)memcpy(output + *out_len, value, value_len);
        *out_len += value_len;
    }
    return IMC_OK;
}

static int tokenize(const char *input, token_t *tokens, size_t *token_count,
                    char *error, size_t error_size)
{
    size_t i = 0U;
    *token_count = 0U;

    while (input[i] != '\0') {
        char word[MAX_TOKEN_LENGTH];
        size_t word_len = 0U;
        char quote = '\0';

        while (isspace((unsigned char)input[i]) != 0) i++;
        if (input[i] == '\0') break;

        if (input[i] == '|') { (void)add_token(tokens, token_count, TOK_PIPE, NULL); i++; continue; }
        if (input[i] == '<') { (void)add_token(tokens, token_count, TOK_IN, NULL); i++; continue; }
        if (input[i] == '&') { (void)add_token(tokens, token_count, TOK_AMP, NULL); i++; continue; }
        if (input[i] == ';') { (void)add_token(tokens, token_count, TOK_SEMI, NULL); i++; continue; }
        if (input[i] == '>') {
            i++;
            if (input[i] == '>') { (void)add_token(tokens, token_count, TOK_APPEND, NULL); i++; }
            else { (void)add_token(tokens, token_count, TOK_OUT, NULL); }
            continue;
        }

        while (input[i] != '\0') {
            char current = input[i];
            if (quote == '\0' && (isspace((unsigned char)current) != 0 ||
                current == '|' || current == '<' || current == '>' || current == '&' || current == ';')) break;
            if ((current == '\'' || current == '"') && (quote == '\0' || quote == current)) {
                quote = (quote == '\0') ? current : '\0';
                i++;
                continue;
            }
            if (current == '\\' && quote != '\'') {
                i++;
                if (input[i] == '\0') break;
                current = input[i];
            } else if (current == '$' && quote != '\'') {
                int result = expand_variable(input, &i, word, &word_len);
                if (result != IMC_OK) {
                    (void)snprintf(error, error_size, "Invalid or oversized environment expansion");
                    return result;
                }
                i++;
                continue;
            }
            if (word_len + 1U >= sizeof(word)) {
                (void)snprintf(error, error_size, "Token exceeds %u characters", MAX_TOKEN_LENGTH - 1U);
                return IMC_LIMIT_ERROR;
            }
            word[word_len++] = current;
            i++;
        }
        if (quote != '\0') {
            (void)snprintf(error, error_size, "Unclosed quote in command");
            return IMC_PARSE_ERROR;
        }
        word[word_len] = '\0';
        if (word_len > 0U && add_token(tokens, token_count, TOK_WORD, word) != IMC_OK) {
            (void)snprintf(error, error_size, "Too many tokens");
            return IMC_LIMIT_ERROR;
        }
    }
    return IMC_OK;
}

int parse_line(char *input, parsed_line_t *parsed, char *error, size_t error_size)
{
    token_t tokens[MAX_ARGS * MAX_PIPE_STAGES * MAX_COMMANDS];
    size_t token_count = 0U;
    size_t i = 0U;
    int result;

    if (input == NULL || parsed == NULL) return IMC_ERROR;
    (void)memset(parsed, 0, sizeof(*parsed));
    if (error != NULL && error_size > 0U) error[0] = '\0';

    result = tokenize(input, tokens, &token_count, error, error_size);
    if (result != IMC_OK) return result;
    if (token_count == 0U) return IMC_OK;

    parsed->command_count = 1U;
    parsed->commands[0].stage_count = 1U;
    (void)snprintf(parsed->commands[0].display, sizeof(parsed->commands[0].display), "%s", input);

    for (i = 0U; i < token_count; i++) {
        pipeline_t *pipeline = &parsed->commands[parsed->command_count - 1U];
        command_stage_t *stage = &pipeline->stages[pipeline->stage_count - 1U];
        token_t *token = &tokens[i];

        if (token->type == TOK_WORD) {
            if (stage->argc >= MAX_ARGS) {
                (void)snprintf(error, error_size, "Maximum argument count (%u) exceeded", MAX_ARGS);
                return IMC_LIMIT_ERROR;
            }
            stage->argv[stage->argc] = strdup(token->text);
            if (stage->argv[stage->argc] == NULL) return IMC_ERROR;
            stage->argc++;
            stage->argv[stage->argc] = NULL;
        } else if (token->type == TOK_PIPE) {
            if (stage->argc == 0U || pipeline->stage_count >= MAX_PIPE_STAGES) {
                (void)snprintf(error, error_size, "Invalid or excessively long pipeline");
                return IMC_PARSE_ERROR;
            }
            pipeline->stage_count++;
        } else if (token->type == TOK_IN || token->type == TOK_OUT || token->type == TOK_APPEND) {
            token_type_t operation = token->type;
            if (i + 1U >= token_count || tokens[i + 1U].type != TOK_WORD) {
                (void)snprintf(error, error_size, "Redirection requires a file name");
                return IMC_PARSE_ERROR;
            }
            i++;
            if (operation == TOK_IN) {
                (void)snprintf(stage->input_file, sizeof(stage->input_file), "%s", tokens[i].text);
            } else {
                (void)snprintf(stage->output_file, sizeof(stage->output_file), "%s", tokens[i].text);
                stage->append_output = (operation == TOK_APPEND);
            }
        } else if (token->type == TOK_AMP) {
            if (i != token_count - 1U) {
                (void)snprintf(error, error_size, "'&' must appear at the end of a command");
                return IMC_PARSE_ERROR;
            }
            pipeline->background = true;
        } else if (token->type == TOK_SEMI) {
            if (stage->argc == 0U || parsed->command_count >= MAX_COMMANDS) {
                (void)snprintf(error, error_size, "Invalid command separator or too many commands");
                return IMC_PARSE_ERROR;
            }
            parsed->command_count++;
            parsed->commands[parsed->command_count - 1U].stage_count = 1U;
            (void)snprintf(parsed->commands[parsed->command_count - 1U].display,
                           sizeof(parsed->commands[0].display), "%s", input);
        }
    }

    for (i = 0U; i < parsed->command_count; i++) {
        if (parsed->commands[i].stages[parsed->commands[i].stage_count - 1U].argc == 0U) {
            (void)snprintf(error, error_size, "Empty command in pipeline");
            return IMC_PARSE_ERROR;
        }
    }
    return IMC_OK;
}
