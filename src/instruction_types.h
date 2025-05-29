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
    uint8_t opc;  // Opcode
    uint8_t rn;   // First operand register (11111: SP)
    uint8_t rd;   // Destination register (11111: SP (or ZR when instr. sets flags))

    // Common fields for SDT and LL instructions
    uint8_t rt;  // Target register

    // DP_IMM specific
    uint8_t opi;  // Operation interpretation
    // Arithmetic (opi == 010)
    uint8_t sh;      // Left-shift imm12 by 12 bits? (0: no, 1: yes)
    uint16_t imm12;  // Immediate value (12 bits) [ALSO USED BY SDT]
    // Wide move (opi == 101)
    uint8_t hw;      // Logical shift left by hw*16 bits
    uint16_t imm16;  // Immediate value (16 bits)

    // DP_REG specific
    uint8_t M;    // Operation interpretation
    uint8_t opr;  // Operation interpretation (decays into shift & N when M == 0)
    uint8_t rm;   // Second operand register
    // Arithmetic and Logical (M == 0)
    uint8_t shift;    // Shift type (0: LSL, 1: LSR, 2: ASR, 3: ROR)
    uint8_t N;        // Bitwise negate the shifted register? (0: no, 1: yes)
    uint8_t operand;  // Immediate value (6 bits, used for shift amount)
    // Multiply (M == 1)
    uint8_t x;   // Negate the product? (0: no, 1: yes)
    uint8_t ra;  // Accumulator register (11111: ZR)

    // SDT specific
    uint8_t U;      // Unsigned offset flag (0: signed, 1: unsigned)
    uint8_t L;      // Transfer type (0: store, 1: load)
    uint8_t xn;     // Base register
    uint8_t xm;     // Offset register
    uint8_t I;      // Post/Pre-indexing (0: post-index, 1: pre-index)
    int16_t simm9;  // Signed immediate offset (sign-extended to 64 bits)
    // uint16_t imm12 is also used here as the immediate value for unsigned offset (U == 1)

    // LL specific
    int64_t simm19;  // Literal offset (sign-extended to 64 bits)

    // BRANCH specific
    int32_t simm26;
    uint8_t cond;
    // Also uses xn and simm19

} DecodedInstruction;

#endif