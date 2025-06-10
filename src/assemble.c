#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include "symbol_table.h"
#include "tokenizer.h"

// TODO: EVERYONE 
// --- TEMPORARY FORWARD DECLARATIONS FOR TEAMMATES' FUNCTIONS ---
// They will implement these in their own files later

uint32_t assembleDataProcessing(char** tokens, int token_count);
uint32_t assembleDataTransfer(char** tokens, int token_count, uint32_t current_address, SymbolTable table);
uint32_t assembleBranch(char** tokens, int token_count, uint32_t current_address, SymbolTable table);
uint32_t assembleDirective(char** tokens, int token_count);

void pass_one(const char* file_in, SymbolTable table);
void pass_two(const char* file_in, const char* file_out, SymbolTable table);

int main(int argc, char **argv) {
  return EXIT_SUCCESS;
}
