#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>

#include "tokenizer.h"

#define SPECIAL_CHARS "[],#!"

// TODO: Zayan read the line below:
// Important: '[' and ']' are received as different tokens - needed for your section

// Helper to check if a character is one of our special single-char tokens
static int is_special_char(char c) {
    return strchr(SPECIAL_CHARS, c) != NULL;
}

char** tokenize(const char* line, int* token_count) {
    *token_count = 0;
    if (line == NULL) {
        return NULL;
    }

    // Allocate initial space for token pointers
    int capacity = 8;
    char** tokens = malloc(capacity * sizeof(char*));
    if (tokens == NULL) {
        perror("malloc for tokens failed");
        return NULL;
    }

    const char* current = line;
    while (*current != '\0') {
        // 1. Skip leading whitespace
        while (isspace((unsigned char)*current)) {
            current++;
        }
        if (*current == '\0') {
            break; // End of line
        }

        const char* token_start = current;
        if (is_special_char(*current)) {
            // 2. Handle special single-character tokens
            current++;
        } else {
            // 3. Handle regular tokens (mnemonics, registers, numbers, labels)
            while (*current != '\0' && !isspace((unsigned char)*current) && !is_special_char(*current)) {
                current++;
            }
        }

        // Extract the token
        int token_len = current - token_start;
        char* new_token = malloc(token_len + 1);
        if (new_token == NULL) {
            perror("malloc for new_token failed");
            free_tokens(tokens, *token_count);
            return NULL;
        }
        strncpy(new_token, token_start, token_len);
        new_token[token_len] = '\0';
        
        // Add token to our list, reallocating if necessary
        if (*token_count >= capacity) {
            capacity *= 2;
            char** new_tokens = realloc(tokens, capacity * sizeof(char*));
            if (new_tokens == NULL) {
                perror("realloc for tokens failed");
                free(new_token);
                free_tokens(tokens, *token_count);
                return NULL;
            }
            tokens = new_tokens;
        }
        tokens[*token_count] = new_token;
        (*token_count)++;
    }

    return tokens;
}

void free_tokens(char** tokens, int token_count) {
    if (tokens == NULL) {
        return;
    }
    for (int i = 0; i < token_count; i++) {
        free(tokens[i]);
    }
    free(tokens);
}