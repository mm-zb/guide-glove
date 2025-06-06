#ifndef MEM_BRANCH_EXECUTOR_H
#define MEM_BRANCH_EXECUTOR_H

#include "addressing.h"
#include "instruction_types.h" // If DecodedInstruction is defined

// Load & Store instruction prototypes
void execute_ldr(ARMState* state, addressing_mode addr_mode, DecodedInstruction* instruction);
void execute_str(ARMState* state, addressing_mode addr_mode, DecodedInstruction* instruction);

// Branch instructions prototypes
void execute_branch_unconditional(ARMState* state, int64_t simm26);
void execute_branch_register(ARMState* state, uint8_t register_xn);
void execute_branch_cond(ARMState* state, int64_t simm19, uint8_t cond);

// Calculating address prototype
uint64_t calculate_address(ARMState* state, addressing_mode addr_mode, DecodedInstruction* instruction);

#endif