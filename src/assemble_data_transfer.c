#include <stdbool.h>

#include "assemble_data_transfer.h"

// Defines shift amounts for each section of the instruction
#define XN_START_BIT 5
#define XM_START_BIT 16
#define IMM12_START_BIT 10
#define SIMM9_START_BIT 12
#define SIMM19_START_BIT 5

// Helper to check the addressing mode class
static bool is_sdt(AddressingMode mode) {
    return (mode != LOAD_LITERAL);
    // If invalid addressing mode this falls into undefined behaviour
    // Pre-condition of valid addressing mode
}

uint32_t assemble_ldr(char** tokens, SymbolTable* symbol_table, uint32_t current_address) {
    uint32_t instruction = 0;
    ParsedAddress address;
    
    AddressingMode mode;
    uint32_t rt;
    uint32_t xn;
    uint32_t xm;
    uint32_t sf;
    uint32_t imm12; 
    int32_t simm9; 
    int32_t simm19;

    // Initialising variables and shifting to correct locations
    address = parse_address(tokens, symbol_table, current_address);
    mode = address.mode;
    rt = address.sdt_ll_reg_rt; // No shift
    xn = address.sdt_reg_xn << XN_START_BIT;
    xm = address.sdt_reg_xm << XM_START_BIT;
    sf = address.sdt_ll_sf; // Boolean so no shift
    imm12 = address.sdt_imm12 << IMM12_START_BIT;
    simm9 = address.sdt_simm9 << SIMM9_START_BIT;
    simm19 = address.ll_simm19 << SIMM19_START_BIT;

    // Constructing binary instruction
    instruction |= rt; 
    instruction |= SDT_L; // As load instruction
    if (is_sdt(mode)) {
        instruction |= SDT_BASE;
        instruction |= xn;
    } else { // Load literal instruction
        instruction |= LL_BASE;
        instruction |= simm19;
    }

    // Handle different addressing modes
    switch (mode) {
        case ZERO_OFFSET:
            // Just want the value in xn and no other offset
            instruction |= SDT_U;
            break;
        case UNSIGNED_OFFSET:
            instruction |= SDT_U;
            instruction |= imm12;
            // Assumes division handled in parse_address function
            break;  
        case PRE_INDEXED:
            instruction |= SDT_I;
            instruction |= SDT_INDEXED;
            instruction |= simm9;
            break;
        case POST_INDEXED:
            instruction |= SDT_INDEXED;
            instruction |= simm9;
            break;
        case REGISTER_OFFSET:
            instruction |= SDT_REG;
            instruction |= xm;
            break;
        case LOAD_LITERAL:
            // Need to break out but behaviour all already handled
            break;
        default:
            // Invalid instruction
            exit(1);
    }

    if (sf) { // 64-bit mode
        instruction |= SDT_LL_SIGN_FLAG;
    }

    return instruction;
}