#ifndef TOKENIZER_H
#define TOKENIZER_H

// Takes a line of assembly and returns a dynamically allocated array of tokens.
// The number of tokens found is stored in token_count.
char** tokenize(const char* line, int* token_count);

// Frees the memory allocated by tokenize().
void free_tokens(char** tokens, int token_count);

#endif