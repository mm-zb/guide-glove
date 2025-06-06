#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>
#include "arm_state.h"
#include "decoder.h"
#include "executor.h"
#include "constants.h"

void load_binary_to_memory(const char* filename, ARMState* state);
void print_final_state(ARMState* state, FILE* output_file);

int main(int argc, char **argv) {
    if (argc < 2 || argc > 3) {
        fprintf(stderr, "Usage: %s <file_in> [file_out]\n", argv[0]);
        return EXIT_FAILURE;
    }

    ARMState arm_state;
    initialize_arm_state(&arm_state);
    load_binary_to_memory(argv[1], &arm_state);

    FILE* output_file = stdout;
    if (argc == 3) {
        output_file = fopen(argv[2], "w");
        if (!output_file) {
            fprintf(stderr, "Error: Could not open output file '%s'\n", argv[2]);
            return EXIT_FAILURE;
        }
    }

    fprintf(stderr, "Starting emulation...\n");
    // Main emulator loop
    uint64_t pc_prev = -1; // Initialize with a value that won't match PC=0
    
    while (arm_state.pc < MEMORY_SIZE) { // Ensure PC doesn't go out of bounds
        if (arm_state.pc % 4 != 0) { // Instructions must be 4-byte aligned
             fprintf(stderr, "Error: PC (0x%016"PRIx64") is not 4-byte aligned. Terminating.\n", arm_state.pc);
             break;
        }

        uint32_t instruction_word = read_word_from_memory(&arm_state, arm_state.pc);
        DecodedInstruction decoded_instr = decode_instruction(instruction_word);

        // Halt condition check (0x8a000000)
        // If it's the halt instruction, stop before execution or PC increment
        // PC will remain at the address of the halt instruction for output
        if (decoded_instr.type == HALT) {
            // The executor function itself will log the halt message.
            break; 
        }
        
        // Execute the instruction. The executor returns true if it handled PC update (e.g. a taken branch).
        // It's vital that if execute_instruction returns true, the PC is not incremented by 4 here.
        bool pc_modified_by_instruction = execute_instruction(&arm_state, &decoded_instr);

        // If the instruction was not a branch, or was a conditional branch not taken,
        // increment PC by 4 for the next instruction.
        if (!pc_modified_by_instruction) {
            arm_state.pc += 4;
        }

        // Infinite loop detection (optional but good for debugging)
        if (pc_prev == arm_state.pc) { 
            fprintf(stderr, "Warning: PC did not advance (0x%016"PRIx64"). Possible infinite loop. Terminating.\n", arm_state.pc);
            break;
        }
        pc_prev = arm_state.pc;
    }
    fprintf(stderr, "Emulation finished.\n");

    print_final_state(&arm_state, output_file);

    if (output_file != stdout) {
        fclose(output_file);
    }

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
        fprintf(output_file, "X%02d = %016"PRIx64"\n", i, state->registers[i]);
    }
    fprintf(output_file, "PC = %016"PRIx64"\n", state->pc);
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

