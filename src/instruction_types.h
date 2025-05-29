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

    uint8_t sf;        // Register size flag (0: 32b, 1: 64b)

    // DP_IMM and DP_REG specific fields
    uint8_t opc;       // Opcode
    uint32_t operand;  // Structure depends on opi
    uint8_t M;         // Op interpretation
    
    uint8_t opi;       // Operation interpretation
    uint8_t opr;       // Op interpretation
    uint8_t rm;        // Second operand register
    uint8_t rn;        // First operand register

    uint8_t rd;        // Destination register

    // SDT and LL specific fields
    uint8_t U;         // Unsigned offset flag (0: signed, 1: unsigned)
    uint8_t L;         // Load flag (0: store, 1: load)
    uint16_t offset;   // Structure depends on U and bit 21
    uint8_t xn;        // Base register
    uint8_t rt;        // Target register
    int64_t simm19;    // Literal offset (sign-extended to 64 bits)

} DecodedInstruction;

#endif