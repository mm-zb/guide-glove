#ifndef INSTRUCTION_TYPES_H
#define INSTRUCTION_TYPES_H

#include <stdint.h>

typedef enum {
    DP_IMM,  // Data Processing with immediate value
    DP_REG,  // Data Processing with register
    SDT,     // Single Data Transfer
    LL,      // Load Literal
    BRANCH,
    HALT,
    UNKNOWN  // Unknown or unrecognized
} InstructionType;

typedef struct {
    uint32_t raw_instruction;
    InstructionType type;

    // Common fields for all instruction types
    uint8_t sf;  // Register size flag (0: 32b, 1: 64b)

    // Common fields for Data Processing instructions
    uint8_t dp_opc;  // Opcode
    uint8_t dp_rn;   // First operand register (11111: SP)
    uint8_t dp_rd;   // Destination register (11111: SP (or ZR when instr. sets flags))

    // DP_IMM specific
    uint8_t dp_imm_opi;  // Operation interpretation
    // Arithmetic (opi == 010)
    uint8_t dp_imm_sh;      // Left-shift imm12 by 12 bits? (0: no, 1: yes)
    uint16_t dp_imm_imm12;  // Immediate value (12 bits)
    // Wide move (opi == 101)
    uint8_t dp_imm_hw;      // Logical shift left by hw*16 bits
    uint16_t dp_imm_imm16;  // Immediate value (16 bits)

    // DP_REG specific
    uint8_t dp_reg_M;    // Operation interpretation
    uint8_t dp_reg_opr;  // Operation interpretation (decays into shift & N when M == 0)
    uint8_t dp_reg_rm;   // Second operand register
    // Arithmetic and Logical (M == 0)
    uint8_t dp_reg_shift_type;    // Shift type (0: LSL, 1: LSR, 2: ASR, 3: ROR)
    uint8_t dp_reg_N;             // Bitwise negate the shifted register? (0: no, 1: yes)
    uint8_t dp_reg_shift_amount;  // Immediate value (6 bits, used for shift amount)
    // Multiply (M == 1)
    uint8_t dp_reg_x;   // Negate the product? (0: no, 1: yes)
    uint8_t dp_reg_ra;  // Accumulator register (11111: ZR)

    // Common fields for SDT and LL instructions
    uint8_t sdt_ll_rt;  // Target register

    // SDT specific
    uint8_t sdt_U;       // Unsigned offset flag (0: signed, 1: unsigned)
    uint8_t sdt_L;       // Transfer type (0: store, 1: load)
    uint8_t sdt_xn;      // Base register
    uint8_t sdt_xm;      // Offset register
    uint8_t sdt_I;       // Post/Pre-indexing (0: post-index, 1: pre-index)
    int16_t sdt_simm9;   // Signed immediate offset (sign-extended to 64 bits)
    uint16_t sdt_imm12;  // Immediate value for unsigned offset (U == 1)

    // LL specific
    int64_t ll_simm19;  // Literal offset (sign-extended to 64 bits)

    // BRANCH specific
    int32_t b_simm26;  // Signed immediate offset (sign-extended to 64 bits)
    uint8_t b_xn;      // Destination address register (11111: ZR)
    int32_t b_simm19;  // Signed immediate offset for conditional jump (sign-extended to 64 bits)
    uint8_t b_cond;    // Condition

} DecodedInstruction;

#endif