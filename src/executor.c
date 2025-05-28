#include "executor.h"
#include <stdio.h>
#include <stdlib.h>

// TODO:
void execute_instruction(ARMState* state, uint32_t instruction_word) {
    // Prasanna and Zayan will implement actual execution logic
    // For now it only handles the halt instruction and increments PC
    
    #ifdef DEBUG
    // Debugging: fprintf(stderr, "Stub: Executing instruction 0x%08x at PC 0x%llx\n", instruction_word, state->pc);
    #endif

    // Halt instruction (0x8a000000) check
    if (instruction_word == HALT_INSTRUCTION) {
        fprintf(stderr, "Halt instruction (0x8a000000) encountered. Terminating emulator.\n");
        // The main loop will handle printing final state after this.
        return; // Signal to main loop to stop
    }

    // Normal PC increment for non-branch instructions
    // Branch instructions will modify PC directly
    state->pc += 4; // This will be the default, branches will override
}