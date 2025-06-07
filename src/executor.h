#ifndef EXECUTOR_H
#define EXECUTOR_H

#include <stdint.h>
#include "arm_state.h"
#include "instruction_types.h"

bool execute_instruction(ARMState* state, DecodedInstruction* instr);

#endif