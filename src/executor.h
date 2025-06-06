#ifndef EXECUTOR_H
#define EXECUTOR_H

#include <stdint.h>
#include "arm_state.h"
#include "instruction_types.h" // If DecodedInstruction is defined

// TODO:
// Function to execute a decoded instruction
// For now, it takes a raw instruction word or a simple decoded form
// Prasanna/Zayan will use the comprehensive DecodedInstruction struct
bool execute_instruction(ARMState* state, uint32_t instruction_word);

#endif