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
uint32_t assembleDirective(char** tokens, int token_count); //TODO: Zayan write this

void pass_one(const char* file_in, SymbolTable table);
void pass_two(const char* file_in, const char* file_out, SymbolTable table);

int main(int argc, char **argv) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <input_file.s> <output_file.bin>\n", argv[0]);
        return EXIT_FAILURE;
    }

    const char* file_in = argv[1];
    const char* file_out = argv[2];

    SymbolTable table = symbol_table_create();
    if (table == NULL) {
        perror("Failed to create symbol table");
        return EXIT_FAILURE;
    }

    // Pass 1: Build the symbol table
    printf("--- Starting Pass 1 ---\n");
    pass_one(file_in, table);
    printf("--- Finished Pass 1 ---\n");

    // Pass 2: Generate the binary code
    printf("--- Starting Pass 2 ---\n");
    pass_two(file_in, file_out, table);
    printf("--- Finished Pass 2 ---\n");

    symbol_table_free(table);
    return EXIT_SUCCESS;
}


void pass_one(const char* file_in, SymbolTable table) {
    FILE *fp = fopen(file_in, "r");
    if (fp == NULL) {
        perror("Error opening input file");
        exit(EXIT_FAILURE);
    }

    char line_buffer[512];
    uint32_t address = 0;

    while (fgets(line_buffer, sizeof(line_buffer), fp)) {
        int token_count = 0;
        char** tokens = tokenize(line_buffer, &token_count);

        if (token_count == 0) { // Skip empty or whitespace-only lines
            free_tokens(tokens, token_count);
            continue;
        }

        // Check if the first token is a label (ends with ':')
        char* first_token = tokens[0];
        int len = strlen(first_token);
        if (first_token[len - 1] == ':') {
            // It's a label so add it to the symbol table with the current address
            first_token[len - 1] = '\0'; // Remove the ':' to get the clean label name
            printf("Found label '%s' at address 0x%x\n", first_token, address);
            symbol_table_add(table, first_token, address);

            // If the line ONLY contained a label, it doesn't take up space
            // If there's an instruction on the same line, we'll increment the address below
            if (token_count == 1) {
                free_tokens(tokens, token_count);
                continue;
            }
        }
        
        // If we reach here, the line contains an instruction or directive that takes up space
        // This could be a standalone instruction, or an instruction after a label
        address += 4; // All instructions and directives in our spec are 4 bytes

        free_tokens(tokens, token_count);
    }
    fclose(fp);
}

void pass_two(const char* file_in, const char* file_out, SymbolTable table) {
    FILE *in_fp = fopen(file_in, "r");
    if (in_fp == NULL) { perror("Error opening input file for Pass 2"); exit(EXIT_FAILURE); }
    
    FILE *out_fp = fopen(file_out, "wb");
    if (out_fp == NULL) { perror("Error opening output file"); fclose(in_fp); exit(EXIT_FAILURE); }

    char line_buffer[512];
    uint32_t address = 0;

    while (fgets(line_buffer, sizeof(line_buffer), in_fp)) {
        int token_count = 0;
        char** tokens = tokenize(line_buffer, &token_count);

        if (token_count == 0) { // Skip empty lines
            free_tokens(tokens, token_count);
            continue;
        }

        // Determine where the actual instruction/directive starts
        // This is important for lines that have a label
        int instruction_token_start_index = 0;
        if (tokens[0][strlen(tokens[0]) - 1] == ':') {
            instruction_token_start_index = 1;
        }

        // If the line was just a label then there's nothing to assemble
        if (instruction_token_start_index >= token_count) {
            free_tokens(tokens, token_count);
            continue;
        }

        // Create pointers to the part of the token list that is the actual instruction
        char** instruction_tokens = &tokens[instruction_token_start_index];
        int instruction_token_count = token_count - instruction_token_start_index;

        const char* mnemonic = instruction_tokens[0];
        uint32_t binary_word = 0;
        
        // TODO: --- Integration Point for Team ---
        if (strcmp(mnemonic, ".int") == 0) {
            // Handle the .int directive directly here.
            binary_word = (uint32_t)strtol(instruction_tokens[1], NULL, 0); // Base 0 auto-detects hex
        } else if (strcmp(mnemonic, "add") == 0 || strcmp(mnemonic, "sub") == 0 /* ... more DP checks */) {
            // binary_word = assembleDataProcessing(instruction_tokens, instruction_token_count);
            binary_word = 0xDEADBEEF; // Placeholder for Richard's work
        } else if (strcmp(mnemonic, "ldr") == 0 || strcmp(mnemonic, "str") == 0) {
            // binary_word = assembleDataTransfer(instruction_tokens, instruction_token_count, address, table);
            binary_word = 0xCAFEBABE; // Placeholder for Zayan's work
        } else if (mnemonic[0] == 'b') {
            // binary_word = assembleBranch(instruction_tokens, instruction_token_count, address, table);
            binary_word = 0xBAADF00D; // Placeholder for Prasanna's work
        } else {
            fprintf(stderr, "Warning: Unknown mnemonic '%s' at address 0x%x. Skipping line.\n", mnemonic, address);
            free_tokens(tokens, token_count);
            continue; // Don't write anything for unknown instructions
        }

        // Write the assembled 32-bit word to the file in little-endian
        fwrite(&binary_word, sizeof(uint32_t), 1, out_fp);
        address += 4;

        free_tokens(tokens, token_count);
    }

    fclose(in_fp);
    fclose(out_fp);
}