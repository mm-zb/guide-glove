#include "branch_assembler.h"
#include <stdio.h>  
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

static uint8_t parse_xn_register(const char* token) {
    assert(token != NULL);

    // Check for XZR
    if (strcmp(token, "xzr") == 0) {
        return 31;
    }

    // Check for Xn registers (e.g., "x0" to "x30")
    if (token[0] == 'x' && token[1] != '\0') {
        char* endptr;
        long reg_num_long = strtol(token + 1, &endptr, 10);

        // Check if conversion was successful and consumed the whole number part
        if (*endptr == '\0' && reg_num_long >= 0 && reg_num_long <= 30) {
            return (uint8_t)reg_num_long;
        }
    }

}

static uint8_t map_condition_to_code(const char* cond_mnemonic) {
    // cond_mnemonic is expected to be like "eq", "ne", etc. (the part after "b.")
    if (strcmp(cond_mnemonic, "eq") == 0) return 0x0; // Z == 1
    if (strcmp(cond_mnemonic, "ne") == 0) return 0x1; // Z == 0
    if (strcmp(cond_mnemonic, "ge") == 0) return 0xA; // N == V
    if (strcmp(cond_mnemonic, "lt") == 0) return 0xB; // N != V
    if (strcmp(cond_mnemonic, "gt") == 0) return 0xC; // Z == 0 AND N == V
    if (strcmp(cond_mnemonic, "le") == 0) return 0xD; // Z == 1 OR N != V
    if (strcmp(cond_mnemonic, "al") == 0) return 0xE; // Always

    fprintf(stderr, "Error (map_condition_to_code): Unknown condition mnemonic '%s'.\n", cond_mnemonic);
    return 0xF; // Invalid condition code, can be used to signal error
}


uint32_t assemble_br_register(char** tokens, int token_count) {
    if (token_count != 2) {
        fprintf(stderr, "Error (assemble_br_register): Expected 2 tokens for 'br Xn' (e.g., br x5), got %d.\n", token_count);
        return 0; // Indicate error (or use a specific error code/exception mechanism)
    }

    // tokens[0] is "br"
    // tokens[1] is Xn (e.g., "x5")

    uint8_t rn_val = parse_xn_register(tokens[1]);
    if (rn_val > 31) { // parse_xn_register returns >31 on error
        // Error already printed by parse_xn_register
        return 0;
    }

    // Bits 31..10 are `1101 0110 0001 1111 0000` (0xD61F0)
    // Bits 9..5 are `Rn`
    // Bits 4..0 are `00000`

    uint32_t instruction = 0;
    instruction |= (0b11010110 << 24); // Bits 31-24 (First 8 bits of the pattern)
    instruction |= (0b00011111 << 16); // Bits 23-16 (Next 8 bits: opc=0000, Res1(part1)=1111) Note: spec diagram has this split differently.


    // The standard encoding for BR Xn is:
    // op0 (bit 25-31) = 1101011
    // op1 (bit 21-24) = 0000 (opc field)
    // op2 (bit 16-20) = 11111 (Reserved, should be 1s)
    // op3 (bit 10-15) = 000000 (Reserved, should be 0s)
    // Rn  (bit 5-9)   = Xn register
    // op4 (bit 0-4)   = 00000 (Reserved, should be 0s)

    instruction = (0b1101011 << 25)  // op0
                | (0b0000 << 21)     // op1 (opc)
                | (0b11111 << 16)    // op2 (Res1)
                | (0b000000 << 10)   // op3 (Res1)
                | (rn_val << 5)      // Rn
                | (0b00000 << 0);    // op4 (Res0)

    return instruction;
}

// Instruction Format (Conditional branch immediate):
// 31-24   | 23-5   | 4 (fixed 0) | 3-0
// 0101010 | simm19 | 0           | cond
uint32_t assemble_b_conditional(char** tokens, int token_count, uint32_t current_address, SymbolTable table) {
    if (token_count != 2) {
        fprintf(stderr, "Error (assemble_b_conditional): Expected 2 tokens for 'b.cond <target>' (e.g., b.eq mylabel), got %d.\n", token_count);
        return 0; // Error
    }

    const char* full_mnemonic = tokens[0]; // e.g., "b.eq"
    const char* target_str = tokens[1];
    uint32_t target_address;
    int32_t offset_bytes;

    // Extract condition from mnemonic
    const char* cond_str_part = strchr(full_mnemonic, '.');
    if (cond_str_part == NULL || *(cond_str_part + 1) == '\0') {
        fprintf(stderr, "Error (assemble_b_conditional): Invalid conditional branch mnemonic format '%s'.\n", full_mnemonic);
        return 0;
    }
    cond_str_part++; // Move past the '.'

    uint8_t cond_code = map_condition_to_code(cond_str_part);
    if (cond_code == 0xF) { // Error from map_condition_to_code
        return 0;
    }

    // Similar to 'b_literal' for resolving target and calculating offset
    if (target_str[0] == '#') {
        char* endptr;
        offset_bytes = (int32_t)strtol(target_str + 1, &endptr, 0);
        if (*endptr != '\0') {
            fprintf(stderr, "Error (assemble_b_conditional): Invalid immediate offset format '%s'.\n", target_str);
            return 0;
        }
    } else {
        if (!symbol_table_get(table, target_str, &target_address)) {
            fprintf(stderr, "Error (assemble_b_conditional): Label '%s' not found in symbol table.\n", target_str);
            return 0; // Error
        }
        offset_bytes = (int32_t)target_address - (int32_t)current_address;
    }

    if (offset_bytes % 4 != 0) {
        fprintf(stderr, "Error (assemble_b_conditional): Branch offset (0x%x bytes) for '%s' is not 4-byte aligned.\n", offset_bytes, target_str);
        return 0;
    }
    int32_t simm19_val = offset_bytes / 4;

    // Bits 31-24: 01010100
    uint32_t opcode_b_cond_fixed_part = 0b01010100;

    uint32_t instruction = (opcode_b_cond_fixed_part << 24)
                         | ((simm19_val & 0x0007FFFF) << 5) // Mask to 19 bits and shift
                         | (cond_code & 0xF);               // Mask to 4 bits

    return instruction;
}