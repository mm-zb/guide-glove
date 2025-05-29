#ifndef INSTRUCTION_TYPES_H
#define INSTRUCTION_TYPES_H

#include <stdint.h>

typedef enum {
		DP_IMM,			 // Data Processing with immediate value
		DP_REG,      // Data Processing with register
		SDT,         // Single Data Transfer
		LL,					 // Load Literal
		BRANCH,     
		HALT,        
		UNKNOWN      // Unknown or unrecognized
} InstructionType;

typedef struct {
     uint32_t raw_instruction;
     InstructionType type;
		 
		 union {
        struct {  // DP Immediate
            uint8_t sf; // Register size flag (0: 32b, 1: 64b)
            uint8_t opc; // Opcode
            uint8_t opi; // Operation interpretation
            uint32_t operand; // Structure depends on opi
            uint8_t rd; // Destination register
        } dp_imm;

        struct {  // DP Register
            uint8_t sf; // Register size flag (0: 32b, 1: 64b)
            uint8_t opc; // Opcode
            uint8_t M; // Op interpretation
            uint8_t opr; // Op interpretation
            uint8_t rm; // Second operand register
            uint8_t operand; // Structure depends on M and opr
            uint8_t rn; // First operand register
            uint8_t rd; // Destination register
        } dp_reg;

        struct {  // Single Data Transfer
            uint8_t sf; // Register size flag (0: 32b, 1: 64b)
						uint8_t U; // Unsigned offset flag (0: signed, 1: unsigned)
            uint8_t L; // Load flag (0: store, 1: load)
            uint16_t offset; // Structure depends on U and bit 21
            uint8_t xn; // Base register
            uint8_t rt; // Target register
        } sdt;

        struct {  // Load Literal
						uint8_t sf; // Register size flag (0: 32b, 1: 64b)
            int32_t simm19; // Literal offset
            uint8_t rt; // Target register
        } load_lit;

        struct {  // Branch
            int32_t operand;
        } branch;
    };
 } DecodedInstruction;

#endif