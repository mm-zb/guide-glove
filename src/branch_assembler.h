#ifndef BRANCH_ASSEMBLER_H
#define BRANCH_ASSEMBLER_H

#include <stdint.h>
#include "symbol_table.h" // To look up label addresses

// tokens: Array of strings from the tokenizer (e.g., {"b", "my_label"})
// token_count: Number of tokens
// current_address: The address of the branch instruction itself (needed for offset calculation)
// table: The symbol table to resolve labels
uint32_t assemble_b_literal(char** tokens, int token_count, uint32_t current_address, SymbolTable table);
uint32_t assemble_br_register(char** tokens, int token_count);
uint32_t assemble_b_conditional(char** tokens, int token_count, uint32_t current_address, SymbolTable table);

// Potentially a main dispatcher if you prefer:
// uint32_t assembleBranchInstruction(char** tokens, int token_count, uint32_t current_address, SymbolTable table);

#endif // BRANCH_ASSEMBLER_H