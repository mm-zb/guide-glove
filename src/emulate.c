#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>
#include <stdbool.h>
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
    
    // Flag to control the main emulation loop
    bool running = true;
    // Store the PC value from the start of the current instruction's execution.
    // Used to detect if the PC advanced after an instruction.
    uint64_t prev_pc = arm_state.pc; 
    
    while (running && arm_state.pc < MEMORY_SIZE) { // Continue as long as 'running' is true and PC is within memory bounds
        // Check if the Program Counter is 4-byte aligned
        if (arm_state.pc % 4 != 0) { 
             fprintf(stderr, "Error: PC (0x%016"PRIx64") is not 4-byte aligned. Terminating.\n", arm_state.pc);
             running = false;
             break; // Exit loop immediately for critical error
        }

        // Fetch the 32-bit instruction word from memory at the current PC
        uint32_t instruction_word = read_word_from_memory(&arm_state, arm_state.pc);
        // Decode the instruction word into its structured representation
        DecodedInstruction decoded_instr = decode_instruction(instruction_word);
        
        // It returns false if the PC should simply be incremented by 4 by the main loop.
        bool pc_was_modified_by_instruction = execute_instruction(&arm_state, &decoded_instr);

        // After executing the instruction, check if it was the HALT instruction.
        // We now set the 'running' flag to false to exit the emulation loop.
        if (decoded_instr.type == HALT) {
            running = false; 
        } 
        // If the instruction did not modify the PC (and it's not HALT), increment PC by 4 to the next instruction.
        else if (!pc_was_modified_by_instruction) {
            arm_state.pc += 4;
        }

        // Detect if the Program Counter has not advanced since the beginning of this instruction's execution.
        // This catches infinite loops like 'b .' (branch to self) where the PC might get stuck.
        // This check is only performed if the emulator is still considered 'running' after the instruction.
        if (running && arm_state.pc == prev_pc) { 
            fprintf(stderr, "Warning: PC did not advance (0x%016"PRIx64"). Possible infinite loop. Terminating.\n", arm_state.pc);
            running = false; // Set flag to stop execution
        }
        
        // Update prev_pc for the next iteration.
        prev_pc = arm_state.pc;
    }
    fprintf(stderr, "Emulation finished.\n");

    print_final_state(&arm_state, output_file);

    if (output_file != stdout) {
        fclose(output_file);
    }

    return EXIT_SUCCESS;
}

void load_binary_to_memory(const char* filename, ARMState* state) {
    FILE* file = fopen(filename, "rb"); 
    if (!file) {
        fprintf(stderr, "Error: Could not open input file '%s'\n", filename);
        exit(EXIT_FAILURE);
    }

    // Read the entire file into memory
    size_t element_size = 1; // Size of each element (1 byte)
    size_t max_elements_to_read = sizeof(state->memory); // Max total bytes to read

    size_t elements_read = fread(state->memory, element_size, max_elements_to_read, file);
    size_t bytes_read_total = elements_read * element_size; // Calculate total bytes read


    if (elements_read == 0 && !feof(file)) { // Check if it's an actual read error, not just empty file
        fprintf(stderr, "Error: Could not read from input file '%s'\n", filename);
        fclose(file);
        exit(EXIT_FAILURE);
    }
    if (bytes_read_total > sizeof(state->memory)) {
        fprintf(stderr, "Warning: Input file '%s' is larger than 2MB memory capacity. Only first 2MB loaded.\n", filename);
    }

    fclose(file);
    fprintf(stderr, "Loaded %zu bytes from '%s' into memory.\n", bytes_read_total, filename);
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
            fprintf(output_file, "0x%08x: %08x\n", addr, word);
        }
    }
}