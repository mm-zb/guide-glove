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
            uint8_t sf;
            uint8_t opc;
            uint8_t opi;
            uint32_t operand;
            uint8_t rd;
        } dp_imm;

        struct {  // DP Register
            uint8_t sf;
            uint8_t opc;
            uint8_t M;
            uint8_t opr;
            uint8_t rm;
            uint8_t operand;
            uint8_t rn;
            uint8_t rd;
        } dp_reg;

        struct {  // Single Data Transfer
            uint8_t sf;
						uint8_t U;
            uint8_t L;
            uint16_t offset;
            uint8_t xn;
            uint8_t rt;
        } sdt;

        struct {  // Load Literal
						uint8_t sf;
            int32_t simm19; 
            uint8_t rt;
        } load_lit;

        struct {  // Branch
            int32_t operand;
        } branch;
    };
 } DecodedInstruction;

#endif