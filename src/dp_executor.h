#ifndef DP_EXECUTOR_H
#define DP_EXECUTOR_H

#include "arm_state.h"         // For ARMState struct
#include "instruction_types.h" // For DecodedInstruction struct

/**
 * @brief Executes a decoded Data Processing (Immediate or Register) instruction.
 *
 * This function is responsible for performing the arithmetic, logical, or
 * move operations specified by the decoded instruction. It updates the
 * ARMState's registers and PSTATE flags accordingly.
 *
 * @param state Pointer to the ARMState structure to be modified.
 * @param instr Pointer to the const DecodedInstruction structure containing
 *              the details of the instruction to execute.
 */
void execute_dp_instruction(ARMState* state, const DecodedInstruction* instr);

// You might add other helper function declarations here if they are specific
// to the overall DP execution logic and need to be called from multiple places
// within dp_executor.c, or if you decide to break down execute_dp_instruction
// into smaller, private static functions within dp_executor.c.

#endif // DP_EXECUTOR_H