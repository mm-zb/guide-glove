#include <string.h>
#include "arm_state.h"

void initialize_arm_state(ARMState* state) {
    // Set all registers to 0
    memset(state->registers, 0, sizeof(state->registers));

    // Set PC to 0
    state->pc = 0;

    // Set memory to 0
    memset(state->memory, 0, sizeof(state->memory));

    // Initialize PSTATE flags
    state->pstate.N = false;
    state->pstate.Z = true; // Z flag is set on startup
    state->pstate.C = false;
    state->pstate.V = false;
}

uint32_t read_word_from_memory(ARMState* state, uint32_t address) {
    if (address + 3 >= sizeof(state->memory)) {
        fprintf(stderr, "Error: Memory access out of bounds at 0x%x\n", address);
        return 0; 
    }

    // A64 instructions are little-endian
    return (uint32_t)state->memory[address] |
           (uint32_t)state->memory[address + 1] << 8 |
           (uint32_t)state->memory[address + 2] << 16 |
           (uint32_t)state->memory[address + 3] << 24;
}

void write_word_to_memory(ARMState* state, uint32_t address, uint32_t value) {
    if (address + 3 >= sizeof(state->memory)) {
        fprintf(stderr, "Error: Memory access out of bounds at 0x%x\n", address);
        return;
    }

    // Convert to Little Endian using masking
    state->memory[address] = (uint8_t)(value & 0xFF);
    state->memory[address + 1] = (uint8_t)((value >> 8) & 0xFF);
    state->memory[address + 2] = (uint8_t)((value >> 16) & 0xFF);
    state->memory[address + 3] = (uint8_t)((value >> 24) & 0xFF);
}