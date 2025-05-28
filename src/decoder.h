#ifndef DECODER_H
#define DECODER_H

#include <stdint.h>
#include "arm_state.h" // For PSTATE flags, if needed by helper functions
#include "instruction_types.h" // If DecodedInstruction is defined

// For now it can return a simple representation or just be void/uint32_t
// Richard will refine this return type to a comprehensive struct
uint32_t decode_instruction(uint32_t instruction_word);

#endif