#ifndef INSTRUCTION_TYPES_H
#define INSTRUCTION_TYPES_H

#include <stdint.h>

typedef struct {

} DecodedInstruction;
// Added this template to allow compilation of mem_branch_executor

// TODO:
// Richard will define structures for decoded instructions here, e.g.:
// typedef struct {
//     uint32_t raw_instruction;
//     InstructionType type; // e.g., DP_IMM, DP_REG, SDT, BRANCH
//     // ... other common fields
// } DecodedInstruction;

#endif