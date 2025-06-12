#include <stdio.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "assemble_dp.h"

// Data processing mnemonic types based on the spec
typedef enum {
    // add, adds, sub, subs
    // <Rd|SP>, <Rn|SP>, #<imm>{, lsl #(0|12)}  OR  <Rd>, <Rn>, <Rm>{, <shift> #<imm>}
    ARITH,
    // cmp, cmn
    // <Rn|SP>, #<imm>{, lsl #(0|12)}  OR  <Rn>, <Rm>{, <shift> #<imm>}
    CMP,
    // neg, negs
    // <Rd|SP>, #<imm>{, lsl #(0|12)}OR <Rd>, <Rm>{, <shift> #<imm>}
    NEG,
    // and, ands, bic, bics, eor, orr, eon, orn
    // <Rd>, <Rn>, <Rm>{, <shift> #<imm>}
    LOGICAL,
    // tst
    // <Rn>, <Rm>{, <shift> #<imm>}
    TST,
    // movk, movn, movz
    // <Rd>, #<imm>{, lsl #<imm>}
    MOVX,
    // mov
    // <Rd>, <Rn>
    MOV,
    // mvn
    // <Rd>, <Rm>{, <shift> #<imm>}
    MVN,
    // madd, msub
    // <Rd>, <Rn>, <Rm>, <Ra>
    M_ARITH,
    // mul, mneg
    // <Rd>, <Rn>, <Rm>
    MUL,
    INVALID,
} mnemonic_type;

typedef struct {
    const char* name;
    mnemonic_type type;
} mnemonic_entry;

// Lookup table to determine mnemonic type
static const mnemonic_entry mnemonic_table[] = {
    {"add", ARITH},   {"adds", ARITH},   {"sub", ARITH},    {"subs", ARITH},   {"cmp", CMP},
    {"cmn", CMP},     {"neg", NEG},      {"negs", NEG},     {"and", LOGICAL},  {"ands", LOGICAL},
    {"bic", LOGICAL}, {"bics", LOGICAL}, {"eor", LOGICAL},  {"orr", LOGICAL},  {"eon", LOGICAL},
    {"orn", LOGICAL}, {"tst", TST},      {"movk", MOVX},    {"movn", MOVX},    {"movz", MOVX},
    {"mov", MOV},     {"mvn", MVN},      {"madd", M_ARITH}, {"msub", M_ARITH}, {"mul", MUL},
    {"mneg", MUL}};

// Forward declarations of helper functions
static void set_bits(uint32_t* target, int start_bit, int end_bit, uint32_t value);
static mnemonic_type get_mnemonic_type(char* mnemonic);
static uint8_t get_register_number(char* reg);
static uint32_t get_immediate_value(char* token);
static uint8_t get_shift_type(char* shift);

// Forward declarations of assembly functions
static uint32_t assemble_arithmetic(char** tokens, int token_count, bool is_64bit);
static uint32_t assemble_cmp(char** tokens, int token_count, bool is_64bit);
static uint32_t assemble_neg(char** tokens, int token_count, bool is_64bit);
static uint32_t assemble_logical(char** tokens, int token_count, bool is_64bit);
static uint32_t assemble_tst(char** tokens, int token_count, bool is_64bit);
static uint32_t assemble_movx(char** tokens, int token_count, bool is_64bit);
static uint32_t assemble_mov(char** tokens, int token_count, bool is_64bit);
static uint32_t assemble_mvn(char** tokens, int token_count, bool is_64bit);
static uint32_t assemble_m_arith(char** tokens, int token_count, bool is_64bit);
static uint32_t assemble_mul(char** tokens, int token_count, bool is_64bit);

uint32_t assembleDataProcessing(char** tokens, int token_count) {
    if (token_count < 3) {
        fprintf(stderr, "Error: Not enough tokens for a data processing instruction.\n");
        return 0;
    }

    char* mnemonic = tokens[0];
    bool is_64bit = tokens[0][0] == 'x';

    // Parse the mnemonic and registers
    switch (get_mnemonic_type(mnemonic)) {
        case ARITH:
            return assemble_arithmetic(tokens, token_count, is_64bit);
        case CMP:
            return assemble_cmp(tokens, token_count, is_64bit);
        case NEG:
            return assemble_neg(tokens, token_count, is_64bit);
        case LOGICAL:
            return assemble_logical(tokens, token_count, is_64bit);
        case TST:
            return assemble_tst(tokens, token_count, is_64bit);
        case MOVX:
            return assemble_movx(tokens, token_count, is_64bit);
        case MOV:
            return assemble_mov(tokens, token_count, is_64bit);
        case MVN:
            return assemble_mvn(tokens, token_count, is_64bit);
        case M_ARITH:
            return assemble_m_arith(tokens, token_count, is_64bit);
        case MUL:
            return assemble_mul(tokens, token_count, is_64bit);
        default:
            fprintf(stderr, "Error: Invalid mnemonic '%s'.\n", mnemonic);
            return 0;
    }
}

// add, adds, sub, subs
static uint32_t assemble_arithmetic(char** tokens, int token_count, bool is_64bit) {
    uint32_t instruction_word = 0;

    uint8_t sf = is_64bit ? 1 : 0;
    uint8_t opc = strncmp(tokens[0], "add", 3) == 0 ? 0 : 2;  // 00 for ADD, 10 for SUB
    if (tokens[0][3] == 's') opc++;                           // 01 for ADDS, 11 for SUBS
    uint8_t rd = get_register_number(tokens[1]);
    uint8_t rn = get_register_number(tokens[2]);

    // Set the instruction word bits
    set_bits(&instruction_word, 31, 31, sf);
    set_bits(&instruction_word, 29, 30, opc);
    // bit 28 (M) is 0 for arithmetic operations
    set_bits(&instruction_word, 5, 9, rn);
    set_bits(&instruction_word, 0, 4, rd);

    // Handle DP_IMM vs DP_REG to set the remaining bits
    bool is_imm = tokens[3][0] == '#';  // Check if the fourth token is an immediate value
    if (is_imm) {
        uint16_t imm12 = get_immediate_value(tokens[3]);
        set_bits(&instruction_word, 26, 28, 3);  // bits 28-26 = 100 hard-coded for DP_IMM
        set_bits(&instruction_word, 23, 25, 2);  // opi = 010 for arithmetic operations
        set_bits(&instruction_word, 10, 21, imm12);
        // If shift provided and shift amount is 12, set the sh bit
        if (token_count == 6 && get_immediate_value(tokens[5]) == 12) {
            set_bits(&instruction_word, 22, 22, 1);
        }
    } else {
        set_bits(&instruction_word, 24, 27, 11);  // bits 27-24 = 1011 for ARITHMETIC DP_REG
        uint8_t rm = get_register_number(tokens[3]);
        set_bits(&instruction_word, 16, 20, rm);
        // Handle shift if provided
        if (token_count == 6) {
            uint8_t shift_type = get_shift_type(tokens[4]);
            uint8_t shift_amt = get_immediate_value(tokens[5]);
            set_bits(&instruction_word, 22, 23, shift_type);  // bits 23-22 = shift type
            set_bits(&instruction_word, 10, 15, shift_amt);
        }
    }

    return instruction_word;
}

// cmp rn, <op2> == subs rzr, rn, <op2>
// cmn rn, <op2> == adds rzr, rn, <op2>
static uint32_t assemble_cmp(char** tokens, int token_count, bool is_64bit) {
    // Populate new tokens to handle the alias
    char* aliased_tokens[token_count + 1];
    aliased_tokens[1] = is_64bit ? "xzr" : "wzr";
    for (int i = 2; i < token_count; i++) aliased_tokens[i] = tokens[i - 1];

    if (strcmp(tokens[0], "cmp") == 0) {
        aliased_tokens[0] = "subs";
        return assemble_arithmetic(aliased_tokens, token_count + 1, is_64bit);
    } else {
        aliased_tokens[0] = "adds";
        return assemble_arithmetic(aliased_tokens, token_count + 1, is_64bit);
    }
}

// neg(s) rd, <op2> == sub(s) rd, rzr, <op2>
static uint32_t assemble_neg(char** tokens, int token_count, bool is_64bit) {
    // Populate new tokens to handle the alias
    char* aliased_tokens[token_count + 1];
    aliased_tokens[1] = tokens[1];
    aliased_tokens[2] = is_64bit ? "xzr" : "wzr";
    for (int i = 3; i < token_count; i++) aliased_tokens[i] = tokens[i - 1];

    if (strcmp(tokens[0], "neg") == 0) {
        aliased_tokens[0] = "sub";
        return assemble_arithmetic(aliased_tokens, token_count + 1, is_64bit);
    } else {
        aliased_tokens[0] = "subs";
        return assemble_arithmetic(aliased_tokens, token_count + 1, is_64bit);
    }
}

// and, ands, bic, bics, orr, orn, eor, eon
static uint32_t assemble_logical(char** tokens, int token_count, bool is_64bit) {
    uint32_t instruction_word = 0;

    uint8_t sf = is_64bit ? 1 : 0;
    uint8_t rd = get_register_number(tokens[1]);
    uint8_t rn = get_register_number(tokens[2]);
    uint8_t rm = get_register_number(tokens[3]);
    // Determine opc and N based on the mnemonic
    // The LSB of opc_N determines N, the rest determines opc
    uint8_t opc_N = 0;
    if (strcmp(tokens[0], "and") == 0)
        opc_N = 0;
    else if (strcmp(tokens[0], "bic") == 0)
        opc_N = 1;
    else if (strcmp(tokens[0], "orr") == 0)
        opc_N = 2;
    else if (strcmp(tokens[0], "orn") == 0)
        opc_N = 3;
    else if (strcmp(tokens[0], "eor") == 0)
        opc_N = 4;
    else if (strcmp(tokens[0], "eon") == 0)
        opc_N = 5;
    else if (strcmp(tokens[0], "ands") == 0)
        opc_N = 6;
    else if (strcmp(tokens[0], "bics") == 0)
        opc_N = 7;
    uint8_t opc = opc_N >> 1;  // Get the opcode bits
    uint8_t N = opc_N & 1;     // Get the N bit

    // Set the instruction word bits
    set_bits(&instruction_word, 31, 31, sf);
    set_bits(&instruction_word, 29, 30, opc);
    // bit 28 (M) is 0 for logical operations
    set_bits(&instruction_word, 25, 27, 5);  // bits 27-25 = 101 for DP_REG
    set_bits(&instruction_word, 21, 21, N);
    set_bits(&instruction_word, 16, 20, rm);
    set_bits(&instruction_word, 5, 9, rn);
    set_bits(&instruction_word, 0, 4, rd);
    // Handle shift if provided
    if (token_count == 6) {
        uint8_t shift_type = get_shift_type(tokens[4]);
        uint8_t shift_amt = get_immediate_value(tokens[5]);
        set_bits(&instruction_word, 22, 23, shift_type);
        set_bits(&instruction_word, 10, 15, shift_amt);
    }

    return instruction_word;
}

// tst rn, <op2> == ands rzr, rn, <op2>
static uint32_t assemble_tst(char** tokens, int token_count, bool is_64bit) {
    // Populate new tokens to handle the alias
    char* aliased_tokens[token_count + 1];
    aliased_tokens[0] = "ands";
    aliased_tokens[1] = is_64bit ? "xzr" : "wzr";
    for (int i = 2; i < token_count; i++) aliased_tokens[i] = tokens[i - 1];

    return assemble_logical(aliased_tokens, token_count + 1, is_64bit);
}

// movn, movz, movk
static uint32_t assemble_movx(char** tokens, int token_count, bool is_64bit) {
    uint32_t instruction_word = 0;  // Set appropriate instruction_word based on mnemonic

    uint8_t sf = is_64bit ? 1 : 0;
    uint8_t rd = get_register_number(tokens[1]);
    uint16_t imm16 = get_immediate_value(tokens[2]);
    uint8_t opc = 0;  // movn
    if (strcmp(tokens[0], "movz") == 0)
        opc = 2;  // opc = 10
    else if (strcmp(tokens[0], "movk") == 0)
        opc = 3;  // opc = 11

    // Set the instruction word bits
    set_bits(&instruction_word, 31, 31, sf);
    set_bits(&instruction_word, 29, 30, opc);
    set_bits(&instruction_word, 26, 28, 4);  // bits 28-26 = 100 for DP_IMM
    set_bits(&instruction_word, 23, 25, 5);  // opi = 101 for movX
    set_bits(&instruction_word, 5, 20, imm16);
    set_bits(&instruction_word, 0, 4, rd);

    // Handle shift if provided
    if (token_count == 5) {
        uint8_t shift_amount = get_immediate_value(tokens[4]);
        uint8_t hw = shift_amount >> 4;
        set_bits(&instruction_word, 21, 22, hw);
    }

    return instruction_word;
}

// mov rd, rm == orr rd, rzr, rm
static uint32_t assemble_mov(char** tokens, int token_count, bool is_64bit) {
    // Populate new tokens to handle the alias
    char* aliased_tokens[token_count + 1];
    aliased_tokens[0] = "orr";
    aliased_tokens[1] = tokens[1];  // rd
    aliased_tokens[2] = is_64bit ? "xzr" : "wzr";
    aliased_tokens[3] = tokens[2];  // rm

    return assemble_logical(aliased_tokens, token_count + 1, is_64bit);
}

// mvn rd, <op2> == orn rd, rzr, <op2>
static uint32_t assemble_mvn(char** tokens, int token_count, bool is_64bit) {
    // Populate new tokens to handle the alias
    char* aliased_tokens[token_count + 1];
    aliased_tokens[0] = "orn";
    aliased_tokens[1] = tokens[1];  // rd
    aliased_tokens[2] = is_64bit ? "xzr" : "wzr";
    for (int i = 3; i < token_count; i++) aliased_tokens[i] = tokens[i - 1];

    return assemble_logical(aliased_tokens, token_count + 1, is_64bit);
}

static uint32_t assemble_m_arith(char** tokens, int token_count) {
    // Example implementation for multiply and arithmetic mnemonics
    uint32_t opcode = 0;  // Set appropriate opcode based on mnemonic
    // Parse tokens and set opcode accordingly
    // This is a placeholder; actual implementation will depend on the specific mnemonic
    return opcode;
}

static uint32_t assemble_mul(char** tokens, int token_count) {
    // Example implementation for multiply mnemonics
    uint32_t opcode = 0;  // Set appropriate opcode based on mnemonic
    // Parse tokens and set opcode accordingly
    // This is a placeholder; actual implementation will depend on the specific mnemonic
    return opcode;
}

// --- HELPER FUNCTIONS ---

static void set_bits(uint32_t* target, int start_bit, int end_bit, uint32_t value) {
    uint32_t res = *target;

    // Clear the bits in the specified range
    uint32_t mask = ((1U << (end_bit - start_bit + 1)) - 1) << start_bit;
    res &= ~mask;

    // Set the bits to the new value
    res |= (value << start_bit) & mask;

    *target = res;  // Update the target with the modified value
}

static mnemonic_type get_mnemonic_type(char* mnemonic) {
    for (int i = 0; i < sizeof(mnemonic_table) / sizeof(mnemonic_entry); i++) {
        if (strcmp(mnemonic, mnemonic_table[i].name) == 0) {
            return mnemonic_table[i].type;
        }
    }
    return INVALID;  // Return INVALID if matching mnemonic not found
}

static uint32_t get_immediate_value(char* token) {
    return strtol(token + 1, NULL, 10);  // Skip the '#' prefix and convert to integer
}

static uint8_t get_register_number(char* reg) {
    if (strcmp(reg + 1, "zr") == 0 || strcmp(reg + 1, "sp") == 0)
        return 31;                     // Special case for ZR/SP
    return strtol(reg + 1, NULL, 10);  // Skip the 'x' or 'w' prefix
}

static uint8_t get_shift_type(char* shift) {
    if (strcmp(shift, "lsl") == 0) return 0;
    if (strcmp(shift, "lsr") == 0) return 1;
    if (strcmp(shift, "asr") == 0) return 2;
    if (strcmp(shift, "ror") == 0) return 3;
    return 4;  // Invalid
}