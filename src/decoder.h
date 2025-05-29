#ifndef DECODER_H
#define DECODER_H

#include <stdint.h>
#include "arm_state.h"
#include "instruction_types.h"

DecodedInstruction decode_instruction(uint32_t instruction_word);
uint32_t get_bits(uint32_t value, uint8_t start, uint8_t end);
InstructionType get_instruction_type(uint32_t instruction_word);

#endif