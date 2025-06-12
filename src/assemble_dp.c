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
    uint32_t opcode = 0;  // Set based on the mnemonic
    uint32_t rd = 0;      // Destination register
    uint32_t rn = 0;      // First operand register
    uint32_t rm = 0;      // Second operand register

    bool is_immediate = tokens[1][0] == '#' || tokens[2][0] == '#';
    bool is_64bit = tokens[0][0] == 'x';

    // Parse the mnemonic and registers
    switch (get_mnemonic_type(mnemonic)) {
        case ARITH:
            opcode = assemble_arithmetic(tokens, token_count);
            break;
        case CMP:
            opcode = assemble_cmp(tokens, token_count);
            break;
        case NEG:
            opcode = assemble_neg(tokens, token_count);
            break;
        case LOGICAL:
            opcode = assemble_logical(tokens, token_count);
            break;
        case TST:
            opcode = assemble_tst(tokens, token_count);
            break;
        case MOVX:
            opcode = assemble_movx(tokens, token_count);
            break;
        case MOV:
            opcode = assemble_mov(tokens, token_count);
            break;
        case MVN:
            opcode = assemble_mvn(tokens, token_count);
            break;
        case M_ARITH:
            opcode = assemble_m_arith(tokens, token_count);
            break;
        case MUL:
            opcode = assemble_mul(tokens, token_count);
            break;
        default:
            fprintf(stderr, "Error: Invalid mnemonic '%s'.\n", mnemonic);
            return 0;
    }

    // Extract registers from tokens
    rd = atoi(tokens[1] + 1);  // Assuming tokens[1] is like "x0"
    rn = atoi(tokens[2] + 1);  // Assuming tokens[2] is like "x1"
    rm = atoi(tokens[3] + 1);  // Assuming tokens[3] is like "x2"

    // Construct the binary instruction
    uint32_t instruction = (opcode << 24) | (rd << 20) | (rn << 16) | rm;

    return instruction;
}

static uint32_t assemble_arithmetic(char** tokens, int token_count) {
    // Example implementation for arithmetic mnemonics
    uint32_t opcode = 0;  // Set appropriate opcode based on mnemonic
    // Parse tokens and set opcode accordingly
    // This is a placeholder; actual implementation will depend on the specific mnemonic
    return opcode;
}

static uint32_t assemble_cmp(char** tokens, int token_count) {
    // Example implementation for comparison mnemonics
    uint32_t opcode = 0;  // Set appropriate opcode based on mnemonic
    // Parse tokens and set opcode accordingly
    // This is a placeholder; actual implementation will depend on the specific mnemonic
    return opcode;
}

static uint32_t assemble_neg(char** tokens, int token_count) {
    // Example implementation for negation mnemonics
    uint32_t opcode = 0;  // Set appropriate opcode based on mnemonic
    // Parse tokens and set opcode accordingly
    // This is a placeholder; actual implementation will depend on the specific mnemonic
    return opcode;
}

static uint32_t assemble_logical(char** tokens, int token_count) {
    // Example implementation for logical mnemonics
    uint32_t opcode = 0;  // Set appropriate opcode based on mnemonic
    // Parse tokens and set opcode accordingly
    // This is a placeholder; actual implementation will depend on the specific mnemonic
    return opcode;
}

static uint32_t assemble_tst(char** tokens, int token_count) {
    // Example implementation for test mnemonics
    uint32_t opcode = 0;  // Set appropriate opcode based on mnemonic
    // Parse tokens and set opcode accordingly
    // This is a placeholder; actual implementation will depend on the specific mnemonic
    return opcode;
}

static uint32_t assemble_movx(char** tokens, int token_count) {
    // Example implementation for move extended mnemonics
    uint32_t opcode = 0;  // Set appropriate opcode based on mnemonic
    // Parse tokens and set opcode accordingly
    // This is a placeholder; actual implementation will depend on the specific mnemonic
    return opcode;
}

static uint32_t assemble_mov(char** tokens, int token_count) {
    // Example implementation for move mnemonics
    uint32_t opcode = 0;  // Set appropriate opcode based on mnemonic
    // Parse tokens and set opcode accordingly
    // This is a placeholder; actual implementation will depend on the specific mnemonic
    return opcode;
}

static uint32_t assemble_mvn(char** tokens, int token_count) {
    // Example implementation for move not mnemonics
    uint32_t opcode = 0;  // Set appropriate opcode based on mnemonic
    // Parse tokens and set opcode accordingly
    // This is a placeholder; actual implementation will depend on the specific mnemonic
    return opcode;
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