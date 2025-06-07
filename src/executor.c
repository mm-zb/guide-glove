#include <stdio.h>
#include <inttypes.h>
#include "executor.h"
#include "dp_executor.h"
#include "mem_branch_executor.h"
#include "decoder.h"


// Returns true if the PC was modified by the instruction (e.g., a taken branch)
// and false otherwise (meaning the main loop should increment PC by 4)
bool execute_instruction(ARMState* state, DecodedInstruction* instr) {
    #ifdef DEBUG
    fprintf(stderr, "Executing instruction type %d at PC 0x%016"PRIx64" (raw: 0x%08x)\n",
            instr->type, state->pc, instr->raw_instruction);
    #endif

    switch (instr->type) {
        case HALT:
            fprintf(stderr, "Halt instruction (0x%08x) encountered. Terminating emulator.\n", HALT_INSTRUCTION);
            return true; // Signal to emulate.c to stop (PC is already at halt address)

        case DP_IMM:
        case DP_REG:
            execute_dp_instruction(state, instr);
            return false; // PC will be incremented by 4 in main loop

        case SDT: {
            // Determine addressing mode for SDT
            addressing_mode mode;
            if (instr->sdt_U) { // Unsigned offset
                mode = UNSIGNED_IMMEDIATE;
            } else if (get_bits(instr->raw_instruction, 21, 21)) { // Register offset
                mode = REGISTER_OFFSET;
            } else { // Pre/Post-indexing
                mode = instr->sdt_I ? PRE_INDEXED : POST_INDEXED;
            }

            if (instr->sdt_L) { // Load
                execute_ldr(state, mode, instr);
            } else { // Store
                execute_str(state, mode, instr);
            }
            return false; // PC will be incremented by 4 in main loop
        }

        case LL: // Load Literal
            execute_ldr(state, LOAD_LITERAL, instr);
            return false; // PC will be incremented by 4 in main loop

        case BRANCH: {
            bool is_branch_taken = false;
            // Compare the two most significant bits (instruction[31:30]) to determine the branch type
            uint32_t branch_group_id = get_bits(instr->raw_instruction, 30, 31); 

            switch (branch_group_id) {
                case 0: {  // Unconditional branch with simm26 offset (pattern 000101 in bits 31-25)
                    execute_branch_unconditional(state, instr->b_simm26);
                    is_branch_taken = true;
                    break;
                }
                case 3: {  // Register branch (pattern 11010110000 in bits 31-21)
                    execute_branch_register(state, instr->b_xn);
                    is_branch_taken = true;
                    break;
                }
                case 1: {  // Conditional branch (pattern 0101010 in bits 31-25)
                    // Evaluate the condition
                    bool condition_met = false;
                    switch (instr->b_cond) {
                        case 0x0: condition_met = state->pstate.Z; break; // EQ
                        case 0x1: condition_met = !state->pstate.Z; break; // NE
                        case 0xA: condition_met = (state->pstate.N == state->pstate.V); break; // GE
                        case 0xB: condition_met = (state->pstate.N != state->pstate.V); break; // LT
                        case 0xC: condition_met = (state->pstate.Z == false && state->pstate.N == state->pstate.V); break; // GT
                        case 0xD: condition_met = !(state->pstate.Z == false && state->pstate.N == state->pstate.V); break; // LE
                        case 0xE: condition_met = true; break; // AL (always)
                        default:
                            fprintf(stderr, "Error: Invalid conditional branch condition 0x%x\n", instr->b_cond);
                            break;
                    }
                    if (condition_met) {
                        execute_branch_unconditional(state, instr->b_simm19);
                        is_branch_taken = true;
                    } else {
                        is_branch_taken = false; // Branch not taken, PC will increment by 4
                    }
                    break;
                }
                default: {
                    fprintf(stderr, "Error: Invalid branch instruction format for group id %u (raw: 0x%08x)\n", branch_group_id, instr->raw_instruction);
                    // This would likely be a fatal error or unrecognized instruction
                    break; 
                }
            }
            return is_branch_taken; // PC was modified by a taken branch, or not (needs +4)
        }

        case UNKNOWN:
        default:
            fprintf(stderr, "Error: Unknown instruction type %d or unhandled instruction 0x%08x at PC 0x%016" PRIx64 "\n",
                (int)instr->type, (unsigned int)instr->raw_instruction, state->pc);
            // This would likely be a fatal error in a real emulator
            return true; // Prevent further execution on unknown instruction to avoid infinite loops
    }
}