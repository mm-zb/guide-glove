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

    char line[512];
    uint32_t address = 0;

    while (fgets(line, sizeof(line), fp)) {
        // Find if there's a ':' for a label
        char* label_end = strchr(line, ':');
        if (label_end != NULL) {
            *label_end = '\0'; // Terminate the string at the colon
            printf("Found label '%s' at address 0x%x\n", line, address);
            symbol_table_add(table, line, address);
            // NOTE: A label can be on the same line as an instruction.
            // For simplicity, we assume labels are on their own line for now.
            // If the rest of the line isn't empty, it's an instruction.
            if (strlen(label_end + 1) > 2) { // check for more than just newline/whitespace
                 address += 4;
            }
        } else {
            // It might be an instruction. Ignore empty/whitespace lines.
            char* trimmed_line = strtok(line, " \t\n");
            if (trimmed_line != NULL) {
                address += 4;
            }
        }
    }
    fclose(fp);
}
