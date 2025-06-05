#include "addressing.h"

// Address = Xn + uoffset
uint64_t get_address_unsigned_immediate(ARMState* state, uint64_t imm12, uint8_t register_xn, uint8_t sf) {
    uint64_t uoffset;
    uint64_t base_address;
    uint64_t target_address;

    if (sf==0) { // Target register is 32-bit
        uoffset = (uint64_t)imm12 << 2;
    } else {
        uoffset = (uint64_t)imm12 << 3;
    } // Cast is just for clarity

    assert(register_xn <= 30);
    // Ensures that xn is a valid register index

    base_address = state->registers[register_xn];
    target_address = base_address + uoffset;
    return target_address;
}

// if I = 1: Address = Xn + simm9
// if I = 0: Address = Xn
uint64_t get_address_indexed(ARMState* state, int64_t simm9, uint8_t register_xn, uint8_t I) {
    uint64_t register_data;
    uint64_t target_address;

    register_data = state->registers[register_xn];
    target_address = register_data + (I ? simm9 : 0);
    state->registers[register_xn] = register_data + simm9;
    
    return target_address;
}

// Address = Xn + Xm
uint64_t get_address_register_offset(ARMState* state, uint8_t register_xm, uint8_t register_xn) {
    uint64_t register_n_data;
    uint64_t register_m_data;
    uint64_t target_address;

    register_n_data = state->registers[register_xn];
    register_m_data = state->registers[register_xm];
    target_address = register_n_data + register_m_data;
    return target_address;
}

// Address = PC + simm19 * 4
uint64_t get_address_load_literal(ARMState* state, int64_t simm19) {
    uint64_t pc_address;
    uint64_t target_address;
    
    pc_address = state->pc;
    target_address = pc_address + (simm19 << 2);
    return target_address;
}
