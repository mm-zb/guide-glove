#include "mem_branch_executor.h"
// addressing.h included implicitly through mem_branch_executor.h
#include "decoder.h"
// Required for get_bits function

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

// Calculates address and moves data into memory, from register rt
void execute_str(ARMState* state, addressing_mode addr_mode, DecodedInstruction* instruction) {
    uint64_t address;
    uint16_t offset;
    uint8_t sf;

    uint8_t register_xn;
    uint8_t register_xm;
    uint8_t register_rt;
    uint64_t register_data;

    int64_t simm9;
    int32_t simm19;

    offset = instruction->offset;
    sf = instruction->sf;
    register_xn = instruction->xn;
    register_rt = instruction->rt;
    register_data = state->registers[register_rt];

    // Common values to all addressing modes

    switch (addr_mode) {
        case (UNSIGNED_IMMEDIATE): 
            address = get_address_unsigned_immediate(state, (uint64_t)offset, register_xn, sf);
            break;
        case (PRE_INDEXED):
            simm9 = (int64_t)get_bits((uint32_t)offset, 12, 20);
            // Takes the correct 9 bits for the offset

            address = get_address_pre_indexed(state, simm9, register_xn, sf);
            break;
        case (POST_INDEXED):
            simm9 = (int64_t)get_bits((uint32_t)offset, 12, 20);
            // Repeated line cannot be avoided

            address = get_address_post_indexed(state, simm9, register_xn, sf);
            break;
        case (REGISTER_OFFSET):
            register_xm = (uint8_t)get_bits((uint32_t)offset, 16, 20);
            address = get_address_register_offset(state, register_xm, register_xn, sf);
            break;
        default:
            // Invalid addressing mode
            // Do nothing for now
            return;
    }

    // TODO: Add GPIO detection
    // TODO: Add conditional write depending on sf
    if (sf == 0) { // Store a 32-bit word

    } else { // Store a 64-bit word

    }

    state->memory[address] = register_data;
    // Temporary to have some effect
}