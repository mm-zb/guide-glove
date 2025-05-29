#include <stdio.h>
#include <string.h>

#include "decoder.h"

uint32_t get_bits(uint32_t value, uint8_t start, uint8_t end);
InstructionType get_instruction_type(uint32_t instruction_word);
int64_t sign_extend(uint32_t value, uint8_t bits);

DecodedInstruction decode_instruction(uint32_t instruction_word) {
    DecodedInstruction i;

    memset(&i, 0, sizeof(DecodedInstruction));  // Initialize all fields to zero
    i.raw_instruction = instruction_word;
    i.type = get_instruction_type(instruction_word);

    // Populate the appropriate fields based on the instruction type
    switch (i.type) {
        case DP_IMM: {
            i.sf = get_bits(instruction_word, 31, 31);
            i.dp_opc = get_bits(instruction_word, 29, 30);
            i.dp_imm_opi = get_bits(instruction_word, 23, 25);
            i.dp_rd = get_bits(instruction_word, 0, 4);

            if (i.dp_imm_opi == 0x2) {  // Arithmetic operation
                i.dp_imm_sh = get_bits(instruction_word, 22, 22);
                i.dp_imm_imm12 = get_bits(instruction_word, 10, 21);
                i.dp_rn = get_bits(instruction_word, 5, 9);
            } else if (i.dp_imm_opi == 0x5) {  // Wide move
                i.dp_imm_hw = get_bits(instruction_word, 21, 22);
                i.dp_imm_imm16 = get_bits(instruction_word, 5, 20);
            }

            break;
        }
        case DP_REG: {
            i.sf = get_bits(instruction_word, 31, 31);
            i.dp_opc = get_bits(instruction_word, 29, 30);
            i.dp_reg_M = get_bits(instruction_word, 28, 28);
            i.dp_reg_opr = get_bits(instruction_word, 21, 24);
            i.dp_reg_rm = get_bits(instruction_word, 16, 20);
            i.dp_reg_shift_amount = get_bits(instruction_word, 10, 15);
            i.dp_rn = get_bits(instruction_word, 5, 9);
            i.dp_rd = get_bits(instruction_word, 0, 4);

            if (i.dp_reg_M) {  // Multiply operation
                i.dp_reg_x = get_bits(instruction_word, 15, 15);
                i.dp_reg_ra = get_bits(instruction_word, 10, 14);
            } else {  // Arithmetic or Logical operation
                i.dp_reg_shift_type = get_bits(instruction_word, 22, 23);
                i.dp_reg_N = get_bits(instruction_word, 21, 21);
            }

            break;
        }
        case SDT: {
            i.sf = get_bits(instruction_word, 30, 30);
            i.sdt_U = get_bits(instruction_word, 24, 24);
            i.sdt_L = get_bits(instruction_word, 22, 22);
            i.sdt_xn = get_bits(instruction_word, 5, 9);
            i.sdt_ll_rt = get_bits(instruction_word, 0, 4);

            if (i.sdt_U) {  // Unsigned offset
                i.sdt_imm12 = get_bits(instruction_word, 10, 21);
            } else if (get_bits(instruction_word, 10, 10)) {  // Pre/Post-indexing
                i.sdt_I = get_bits(instruction_word, 11, 11);
                i.sdt_simm9 = sign_extend(get_bits(instruction_word, 12, 20), 9);
            } else {  // Register offset
                i.sdt_xm = get_bits(instruction_word, 16, 20);
            }

            break;
        }
        case LL: {
            i.sf = get_bits(instruction_word, 30, 30);
            // Sign-extend to 64 bits
            i.ll_simm19 = sign_extend(get_bits(instruction_word, 5, 23), 19);
            i.sdt_ll_rt = get_bits(instruction_word, 0, 4);
            break;
        }
        case BRANCH: {
            // Compare the two most significant bits to determine the branch type
            switch (get_bits(instruction_word, 30, 31)) {
                case 0: {  // Unconditional branch (00)
                    i.b_simm26 = sign_extend(get_bits(instruction_word, 0, 25), 26);
                    i.b_xn = get_bits(instruction_word, 5, 9);
                    break;
                }
                case 3: {  // Register branch (11)
                    i.b_xn = get_bits(instruction_word, 5, 9);
                    break;
                }
                case 1: {  // Conditional branch (01)
                    i.b_cond = get_bits(instruction_word, 0, 3);
                    i.b_simm19 = sign_extend(get_bits(instruction_word, 5, 23), 19);
                    break;
                }
                default: {
                    fprintf(stderr, "Error: Invalid branch instruction format\n");
                    break;
                }
            }
            break;
        }
        case HALT:
        case UNKNOWN: {
            // No additional fields to decode for HALT or UNKNOWN
            break;
        }
        default: {
            fprintf(stderr, "Error: Unknown instruction type %d\n", i.type);
            break;
        }
    }
    return i;
}

uint32_t get_bits(uint32_t value, uint8_t start, uint8_t end) {
    // Extract bits from start index to end index (inclusive)
    return (value >> start) & ((1U << (end - start + 1)) - 1);
}

InstructionType get_instruction_type(uint32_t instruction_word) {
    // Special case for the HALT instruction (defined in constants.h)
    if (instruction_word == HALT_INSTRUCTION) return HALT;

    uint32_t op0 = get_bits(instruction_word, 25, 28);

    // Mask out don't care bits and compare against given patterns
    // op0:     Group:
    // 100x    Data Processing (Immediate)
    // x101    Data Processing (Register)
    // x1x0    Loads and Stores
    // 101x    Branches
    if ((op0 & 0xE) == 0x8) {
        return DP_IMM;
    } else if ((op0 & 0x7) == 0x5) {
        return DP_REG;
    } else if ((op0 & 0x5) == 0x4) {
        // SDT/LL share the same op0 pattern, use the MSB to differentiate
        return (instruction_word & (1 << 31)) ? SDT : LL;
    } else if ((op0 & 0xE) == 0xA) {
        return BRANCH;
    } else {
        return UNKNOWN;
    }
}

int64_t sign_extend(uint32_t value, uint8_t bits) {
    // Sign-extend the first `bits` bits of `value` to 64b
    if (value & (1U << (bits - 1))) {
        // If the sign bit is set, extend the sign
        return value | ((int64_t)-1 << bits);
    } else {
        // If the sign bit is not set, just return the value
        return value;
    }
}