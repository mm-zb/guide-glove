#include "mem_branch_executor.h"
// addressing.h included implicitly through mem_branch_executor.h
#include "decoder.h"
// Required for get_bits function

#define GPIO_BASE 0x3f200000
#define GPIO_END 0x3f200030

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
    // const int EQ = 0x0;
    // const int NE = 0x1;
    // const int GE = 0xA;
    // const int LT = 0xB;
    // const int GT = 0xC;
    // const int LE = 0xD;
    // const int AL = 0xE;

    // Breaking out of the switch statement is the same as branching
    // Returning is the same as leaving the PC unchanged
    switch (cond) {
        case 0x0:
            // (state->pstate.Z) ? break : return;
            // TODO: Change these if statements for one-liners somehow (as above)
            if (state->pstate.Z) break;
            return;
        case 0x1:
            if (!state->pstate.Z) break;
            return;
        case 0xA:
            if (state->pstate.N == state->pstate.V) break;
            return;
        case 0xB:
            if (state->pstate.N != state->pstate.V) break;
            return;
        case 0xC:
            if (state->pstate.Z==0 && state->pstate.N == state->pstate.V) break;
            return;
        case 0xD:
            if (!(state->pstate.Z==0 && state->pstate.N == state->pstate.V)) break;
            return;
        case 0xE:
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
    uint8_t sf;

    uint8_t register_rt;
    uint64_t register_data;

    int bytes_stored;

    address = calculate_address(state, addr_mode, instruction);
    sf = instruction->sf;

    register_rt = instruction->sdt_ll_rt;
    register_data = state->registers[register_rt];

    if ((address >= GPIO_BASE) && (address <= GPIO_END)) {
        // Don't write to memory
        // TODO: Add GPIO updating
        return;
    }

    // Conditional write depending on sf
    else if (sf == 0) { // Store a 32-bit word
        bytes_stored = 4;
    } else { // Store a 64-bit doubleword
        bytes_stored = 8;
    }
    for (int i=0; i<bytes_stored; i++) {
        state->memory[address + i] = (register_data >> 8*i) & 0xFF;
    }
}

// Calculates address and moves data into register rt, from memory
void execute_ldr(ARMState* state, addressing_mode addr_mode, DecodedInstruction* instruction) {
    uint64_t address;
    uint8_t sf;
    int32_t simm19;

    uint8_t register_rt;
    uint64_t data_to_store = 0;

    int bytes_stored;

    sf = instruction->sf;

    if (addr_mode == LOAD_LITERAL) {
        simm19 = (int32_t)instruction->ll_simm19;
        address = get_address_load_literal(state, simm19, sf);
    } else {
        address = calculate_address(state, addr_mode, instruction);
    }
    
    register_rt = instruction->sdt_ll_rt;

    // Conditional write depending on sf
    if (sf == 0) { // Store a 32-bit word
        bytes_stored = 4;
    } else { // Store a 64-bit doubleword
        bytes_stored = 8;
    }
    for (int i=0; i<bytes_stored; i++) {
        data_to_store |= (uint64_t)state->memory[address + i] << (8 * i);
    }
    state->registers[register_rt] = data_to_store;
}

// Calculates address with addressing mode
uint64_t calculate_address(ARMState* state, addressing_mode addr_mode, DecodedInstruction* instruction) {
    uint64_t address;
    uint16_t offset;
    uint8_t sf;
    uint8_t I;

    uint8_t register_xn;
    uint8_t register_xm;

    int64_t simm9;

    offset = instruction->sdt_imm12;
    register_xn = instruction->sdt_xn;
    sf = instruction->sf;

    switch (addr_mode) {
    case (UNSIGNED_IMMEDIATE): 
        address = get_address_unsigned_immediate(state, (uint64_t)offset, register_xn, sf);
        break;
    case (PRE_INDEXED):
    case (POST_INDEXED):
        simm9 = (int64_t)get_bits((uint32_t)offset, 12, 20);
        I = (uint8_t)instruction->sdt_I;
        // Takes the correct 9 bits for the offset and the index flag I
        address = get_address_indexed(state, simm9, register_xn, sf, I);
        break;
    case (REGISTER_OFFSET):
        register_xm = (uint8_t)get_bits((uint32_t)offset, 16, 20);
        address = get_address_register_offset(state, register_xm, register_xn, sf);
        break;
    default:
        // Invalid addressing mode
        // Do nothing for now
        fprintf(stderr, "Invalid addressing mode!");
        return 0;
    }
    return address;
}