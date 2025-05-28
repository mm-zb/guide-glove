#ifndef ARM_STATE_H
#define ARM_STATE_H

#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include "constants.h"

// ARMv8 machine state
typedef struct {
    uint64_t registers[31]; // X0-X30 general-purpose registers
    uint64_t pc;            // Program Counter

    // Processor State Register (PSTATE) flags
    struct {
        bool N; // Negative
        bool Z; // Zero
        bool C; // Carry
        bool V; // Overflow
    } pstate;

    // 2MB byte-addressable memory
    uint8_t memory[MEMORY_SIZE]; 
} ARMState;

// Common functions
void initialize_arm_state(ARMState* state);
uint32_t read_word_from_memory(ARMState* state, uint32_t address);
void write_word_to_memory(ARMState* state, uint32_t address, uint32_t value);

// Might add basic get/set register functions here, or let others access directly
// uint64_t get_register(ARMState* state, int reg_id);
// void set_register(ARMState* state, int reg_id, uint64_t value);

#endif