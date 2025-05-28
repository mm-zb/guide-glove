#include "arm_state.h"
#include <string.h>

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