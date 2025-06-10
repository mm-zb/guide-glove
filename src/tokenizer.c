#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "tokenizer.h"

// Define delimiters that separate tokens
// This is a crucial part - we want to keep some special chars as tokens themselves
// A simpler approach for now is to use whitespace and commas as separators
// This file may need to refine this for addresses like [r1, #4]!

#define DELIMS " ,\t\n"

char** tokenize(const char* line, int* token_count) {
    char* line_copy = strdup(line);
    if (!line_copy) {
        perror("strdup failed in tokenizer");
        *token_count = 0;
        return NULL;
    }

    // First pass to count tokens
    int count = 0;
    char* temp_copy = strdup(line); // Need a second copy for counting
    for (char* token = strtok(temp_copy, DELIMS); token != NULL; token = strtok(NULL, DELIMS)) {
        count++;
    }
    free(temp_copy);

    *token_count = count;
    if (count == 0) {
        free(line_copy);
        return NULL;
    }

    // Allocate memory for the array of token pointers
    char** tokens = malloc(count * sizeof(char*));
    if (!tokens) {
        perror("malloc for tokens failed");
        free(line_copy);
        *token_count = 0;
        return NULL;
    }

    // Second pass to actually get and store tokens
    int i = 0;
    for (char* token = strtok(line_copy, DELIMS); token != NULL; token = strtok(NULL, DELIMS)) {
        tokens[i++] = strdup(token);
    }
    
    // strtok modifies line_copy, so we free the original pointer. 
    // The memory for the tokens themselves is now pointed to by our 'tokens' array.
    free(line_copy);

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