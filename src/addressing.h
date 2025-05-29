#ifndef ADDRESSING_H
#define ADDRESSING_H

#include "arm_state.h"

typedef enum {
    UNSIGNED_IMMEDIATE,
    PRE_INDEXED,
    POST_INDEXED,
    REGISTER_OFFSET,
    LOAD_LITERAL,
} addressing_mode;

// Addressing mode prototypes
// Each function here takes the number of the register (X0-30) rather than its contents
// Each argument has been bit extended to 64 bits
// Unsigned Immediate (U = 1)
uint64_t get_address_unsigned_immediate(ARMState* state, uint64_t imm12, uint8_t register_xn, uint8_t sf);
// Pre-Indexed (I = 1)
uint64_t get_address_pre_indexed(ARMState* state, int64_t simm9, uint8_t register_xn, uint8_t sf);
// Post-Indexed (I = 0)
uint64_t get_address_post_indexed(ARMState* state, int64_t simm9, uint8_t register_xn, uint8_t sf);
// Register Offset 
uint64_t get_address_register_offset(ARMState* state, uint8_t register_xm, uint8_t register_xn, uint8_t sf);
// Load Literal
uint64_t get_address_load_literal(ARMState* state, int32_t simm19, uint8_t sf);

#endif