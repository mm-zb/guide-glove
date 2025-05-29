#include "decoder.h"

#include <stdio.h>

DecodedInstruction decode_instruction(uint32_t instruction_word) {
    DecodedInstruction i;

    memset(&i, 0, sizeof(DecodedInstruction));  // Initialize all fields to zero
    i.raw_instruction = instruction_word;
    i.type = get_instruction_type(instruction_word);

    switch (i.type) {
        case DP_IMM: {
            i.sf = get_bits(instruction_word, 31, 31);
            i.opc = get_bits(instruction_word, 29, 30);
            i.opi = get_bits(instruction_word, 23, 25);
            i.operand = get_bits(instruction_word, 10, 21);
            i.rd = get_bits(instruction_word, 0, 4);
            break;
        }
        case DP_REG: {
            i.sf = get_bits(instruction_word, 31, 31);
            i.opc = get_bits(instruction_word, 29, 30);
            i.opr = get_bits(instruction_word, 21, 24);
            i.rm = get_bits(instruction_word, 16, 20);
            i.operand = get_bits(instruction_word, 10, 15);
            i.rn = get_bits(instruction_word, 5, 9);
            i.rd = get_bits(instruction_word, 0, 4);
            break;
        }
        case SDT: {
            i.sf = get_bits(instruction_word, 30, 30);
            i.U = get_bits(instruction_word, 24, 24);
            i.L = get_bits(instruction_word, 22, 22);
            i.offset = get_bits(instruction_word, 10, 21);
            i.xn = get_bits(instruction_word, 5, 9);
            i.rt = get_bits(instruction_word, 0, 4);
            break;
        }
        case LL: {
            i.sf = get_bits(instruction_word, 30, 30);
            // sign extend to 64
            i.simm19 = ((int64_t)get_bits(instruction_word, 5, 21) << 13) >> 13;
            i.rt = get_bits(instruction_word, 0, 4);
            break;
        }
        case BRANCH: {
            i.operand = get_bits(instruction_word, 0, 25);
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

uint32_t get_bits(uint32_t value, int start, int end) {
    // Extract bits from start to end index (inclusive)
    return (value >> start) & ((1U << (end - start + 1)) - 1);
}

InstructionType get_instruction_type(uint32_t instruction_word) {
    // Special case for the HALT instruction (defined in constants.h)
    if (instruction_word == HALT_INSTRUCTION) return HALT;

    uint32_t op0 = get_bits(instruction_word, 25, 28);

    // Mask out don't care bits and compare against given patterns
    if ((op0 & 0b1110) == 0b1000) {
        return DP_IMM;
    } else if ((op0 & 0b0111) == 0b0101) {
        return DP_REG;
    } else if ((op0 & 0b0101) == 0b0100) {
        // SDT/LL share the same op0 pattern, use the MSB to differentiate
        return (instruction_word & (1 << 31)) ? SDT : LL; 
    } else if ((op0 & 0b1110) == 0b1010) {
        return BRANCH;
    } else {
        return UNKNOWN;
    }
}