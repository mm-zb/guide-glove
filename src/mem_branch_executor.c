#include "mem_branch_executor.h"
// addressing.h included implicitly through mem_branch_executor.h

// PC = PC + 4 * simm26
void execute_branch_unconditional(ARMState* state, int64_t simm26) {
    uint64_t next_pc;
    uint64_t curr_pc;
    int64_t offset; // Signed

    offset = simm26 << 2;
    curr_pc = state->pc;
    next_pc = curr_pc + offset;
    state->pc = next_pc;
}

// PC = Xn
void execute_branch_register(ARMState* state, uint8_t register_xn) {
    uint8_t xzr = 0x1F;
    uint64_t next_pc;
    uint64_t register_data;

    if (register_xn == xzr) {
        // We are at the zero register, we do not need to handle this case
        // So abort early and the PC remains unchanged
        // This might need to set the PC to zero but I am unsure for now
        return;
    }

    register_data = state->registers[register_xn];
    next_pc = register_data;
    state->pc = next_pc;
}