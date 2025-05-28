#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "arm_state.h"
#include "decoder.h"
#include "executor.h"

int main(int argc, char **argv) {
  return EXIT_SUCCESS;
}

void load_binary_to_memory(const char* filename, ARMState* state) {
    // Open in binary read mode
    FILE* file = fopen(filename, "rb"); 
    if (!file) {
        fprintf(stderr, "Error: Could not open input file '%s'\n", filename);
        exit(EXIT_FAILURE);
    }

    // Read the entire file into memory
    size_t bytes_read = fread(state->memory, 1, sizeof(state->memory), file);
    if (bytes_read == 0 && !feof(file)) { // Check if it's an actual read error, not just empty file
        fprintf(stderr, "Error: Could not read from input file '%s'\n", filename);
        fclose(file);
        exit(EXIT_FAILURE);
    }
    if (bytes_read > sizeof(state->memory)) {
        fprintf(stderr, "Warning: Input file '%s' is larger than 2MB memory capacity. Only first 2MB loaded.\n", filename);
    }

    fclose(file);
    fprintf(stderr, "Loaded %zu bytes from '%s' into memory.\n", bytes_read, filename);
}

void print_final_state(ARMState* state, FILE* output_file) {
    fprintf(output_file, "Registers:\n");
    for (int i = 0; i < 31; ++i) {
        fprintf(output_file, "X%02d = %016llx\n", i, state->registers[i]);
    }
    fprintf(output_file, "PC = %016llx\n", state->pc); // Use %llx for uint64_t
    fprintf(output_file, "PSTATE : %c%c%c%c\n",
            state->pstate.N ? 'N' : '-',
            state->pstate.Z ? 'Z' : '-',
            state->pstate.C ? 'C' : '-',
            state->pstate.V ? 'V' : '-');

    fprintf(output_file, "Non-zero memory:\n");
    for (uint32_t addr = 0; addr < sizeof(state->memory); addr += 4) {
        // Read a 32-bit word, then check if it's non-zero
        uint32_t word = read_word_from_memory(state, addr);
        if (word != 0) {
            fprintf(output_file, "0x%08x: 0x%08x\n", addr, word);
        }
    }
}

