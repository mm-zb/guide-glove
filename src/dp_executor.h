#ifndef DP_EXECUTOR_H
#define DP_EXECUTOR_H

#include "arm_state.h"         // For ARMState struct
#include "instruction_types.h" // For DecodedInstruction struct
#include "shifts.h"            // For ShiftType enum and execute_shift function

//Executes a decoded Data Processing (Immediate or Register) instruction.
void execute_dp_instruction(ARMState* state, const DecodedInstruction* instr);

#endif