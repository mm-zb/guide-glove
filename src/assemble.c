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

    char line[512];
    uint32_t address = 0;

    while (fgets(line, sizeof(line), in_fp)) {
        char* label_end = strchr(line, ':');
        char* instruction_start = line;
        if (label_end != NULL) {
            instruction_start = label_end + 1;
        }

        int token_count = 0;
        char** tokens = tokenize(instruction_start, &token_count);

        if (token_count == 0) { continue; } // Skip empty lines

        const char* mnemonic = tokens[0];
        uint32_t instruction_binary = 0;
        bool is_instruction = true;

        // TODO: 
        // --- Integration Point for Team ---
        if (strcmp(mnemonic, "add") == 0 || strcmp(mnemonic, "sub") == 0) { // Add other DP instructions here
            // instruction_binary = assembleDataProcessing(tokens, token_count);
            // This is just a placeholder to test the framework
            instruction_binary = 0xDEADBEEF; // Placeholder for Richard's work
        } else if (strcmp(mnemonic, "ldr") == 0 || strcmp(mnemonic, "str") == 0) {
            // instruction_binary = assembleDataTransfer(tokens, token_count, address, table);
            instruction_binary = 0xCAFEBABE; // Placeholder for other Richard's work
        } else if (mnemonic[0] == 'b') {
            // instruction_binary = assembleBranch(tokens, token_count, address, table);
            instruction_binary = 0xBAADF00D; // Placeholder for Prasanna's work
        } else if (strcmp(mnemonic, ".int") == 0) {
            // instruction_binary = assembleDirective(tokens, token_count);
            is_instruction = false;
        } else {
            fprintf(stderr, "Warning: Unknown mnemonic '%s' at address 0x%x\n", mnemonic, address);
            is_instruction = false;
        }

        if (is_instruction) {
             // Write the assembled instruction to the file (little-endian is default on x86)
            fwrite(&instruction_binary, sizeof(uint32_t), 1, out_fp);
            address += 4;
        }

        free_tokens(tokens, token_count);
    }

    fclose(in_fp);
    fclose(out_fp);
}