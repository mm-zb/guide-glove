#ifndef DECODER_H
#define DECODER_H

#include <stdint.h>
#include "instruction_types.h"

DecodedInstruction decode_instruction(uint32_t instruction_word);

#endif