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

// If cond then PC = PC + 4 * simm19
void execute_branch_cond(ARMState* state, int64_t simm19, uint8_t cond) {
    const EQ = 0x0;
    const NE = 0x1;
    const GE = 0xA;
    const LT = 0xB;
    const GT = 0xC;
    const LE = 0xD;
    const AL = 0xE;

    // Breaking out of the switch statement is the same as branching
    // Returning is the same as leaving the PC unchanged
    switch (cond) {
        case EQ:
            // (state->pstate.Z) ? break : return;
            // TODO: Change these if statements for one-liners somehow (as above)
            if (state->pstate.Z) break;
            return;
        case NE:
            if (!state->pstate.Z) break;
            return;
        case GE:
            if (state->pstate.N == state->pstate.V) break;
            return;
        case LT:
            if (state->pstate.N != state->pstate.V) break;
            return;
        case GT:
            if (state->pstate.Z==0 && state->pstate.N == state->pstate.V) break;
            return;
        case LE:
            if (!(state->pstate.Z==0 && state->pstate.N == state->pstate.V)) break;
            return;
        case AL:
            break;
        default:
            // Invalid condition code
            // For now I will leave the PC unchanged
            return;
    }

    execute_branch_unconditional(state, simm19);
}