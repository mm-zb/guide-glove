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