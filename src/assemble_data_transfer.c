#include <stdbool.h>
#include <stdlib.h>
#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "assemble_data_transfer.h"

// Defines macro for error reporting
#define STRINGIFY(var) #var

// Defines shift amounts for each section of the instruction
#define XN_START_BIT 5
#define XM_START_BIT 16
#define IMM12_START_BIT 10
#define SIMM9_START_BIT 12
#define SIMM19_START_BIT 5

// Helper to check the addressing mode class
static bool is_sdt(AddressingMode mode) {
    return (mode != LOAD_LITERAL);
    // If invalid addressing mode this falls into undefined behaviour
    // Pre-condition of valid addressing mode
}

uint32_t assemble_ldr(char** tokens, int token_count, SymbolTable symbol_table, uint32_t current_address) {
    uint32_t instruction = 0;
    instruction = assemble_loadstore(tokens, token_count, symbol_table, current_address, true); 
    // is_ldr is set to true
    return instruction;
}

uint32_t assemble_str(char** tokens, int token_count, SymbolTable symbol_table, uint32_t current_address) {
    uint32_t instruction = 0;
    instruction = assemble_loadstore(tokens, token_count, symbol_table, current_address, false); 
    // is_ldr is set to false
    return instruction;
}

uint32_t assemble_loadstore(char** tokens, int token_count, SymbolTable symbol_table, uint32_t current_address, bool is_ldr) {

    uint32_t instruction = 0;
    ParsedAddress address;
    
    AddressingMode mode;
    uint32_t rt;
    uint32_t xn;
    uint32_t xm;
    uint32_t sf;
    uint32_t imm12; 
    uint32_t simm9; // Unsigned as we want our shifting to be logical not arithmetic
    uint32_t simm19;

    // Initialising variables and shifting to correct locations
    address = parse_address(tokens, token_count, symbol_table, current_address);
    mode = address.mode;
    rt = address.sdt_ll_reg_rt; // No shift
    xn = address.sdt_reg_xn << XN_START_BIT;
    xm = address.sdt_reg_xm << XM_START_BIT;
    sf = address.sdt_ll_sf; // Boolean so no shift
    imm12 = address.sdt_imm12 << IMM12_START_BIT;
    simm9 = address.sdt_simm9 << SIMM9_START_BIT;
    simm19 = address.ll_simm19 << SIMM19_START_BIT;

    // Constructing binary instruction
    instruction |= rt; 
    if (is_ldr) {
        instruction |= SDT_L; // Add load bit
    }
    if (is_sdt(mode)) {
        instruction |= SDT_BASE;
        instruction |= xn;
    } else { // Load literal instruction
        instruction |= LL_BASE;
        instruction |= simm19;
    }

    // Handle different addressing modes
    switch (mode) {
        case ZERO_OFFSET:
            // Just want the value in xn and no other offset
            instruction |= SDT_U;
            break;
        case UNSIGNED_OFFSET:
            instruction |= SDT_U;
            instruction |= imm12;
            // Assumes division handled in parse_address function
            break;  
        case PRE_INDEXED:
            instruction |= SDT_I;
            instruction |= SDT_INDEXED;
            instruction |= simm9;
            break;
        case POST_INDEXED:
            instruction |= SDT_INDEXED;
            instruction |= simm9;
            break;
        case REGISTER_OFFSET:
            instruction |= SDT_REG;
            instruction |= xm;
            break;
        case LOAD_LITERAL:
            // Need to break out but behaviour all already handled
            break;
        default:
            // Invalid instruction
            exit(1);
    }

    if (sf) { // 64-bit mode
        instruction |= SDT_LL_SIGN_FLAG;
    }

    return instruction;
}

uint32_t assemble_directive(char** tokens, int token_count, SymbolTable symbol_table, uint32_t current_address) {
    //tokens = [".int", x]
    assert(strcmp(tokens[0], ".int") == 0);
    assert(token_count == 2);

    const char *value = tokens[1]; // Designating as const to keep read only
    uint32_t directive_value;
    if (!symbol_table_get(symbol_table, value, &directive_value)) {
        // Not in symbol table, so numeric literal
        if (strlen(value) > 2 && value[0] == '0' && (value[1] == 'x' || value[1] == 'X')) {
            // Hex so need to ignore the "0x" prefix of the string, and then convert
            directive_value = (uint32_t)strtoul(value + 2, NULL, 16);
        } else {
            // Assume decimal (and not a different base system)
            directive_value = (uint32_t)atoi(value);
        }
    }

    return directive_value;
}

AddressingMode get_addressing_mode(char** tokens, int token_count, int lbrace, int rbrace, int hashtag) {
    AddressingMode mode; 

    if (lbrace == -1) { // No brackets found
        mode = LOAD_LITERAL;
    } else if (rbrace + 1 < token_count && strcmp(tokens[rbrace + 1], "!") == 0) {
        mode = PRE_INDEXED;
    } else if (rbrace == token_count - 1) { // Unsigned immediate or register
        // Check if hashtag in our function
        bool contains_hashtag = false;

        // We have the precondition that lbrace < hashtag < rbrace 
        // as it was asserted before this function was called
        for (int i = lbrace; i < rbrace; i++) {
            if (strcmp(tokens[i], "#") == 0) {
                contains_hashtag = true;
            } 
        }

        contains_hashtag ? (mode = UNSIGNED_OFFSET) : (mode = REGISTER_OFFSET);
    } else if (rbrace + 1 < token_count && strcmp(tokens[rbrace + 1], ",") == 0 &&
               rbrace + 2 < token_count && strcmp(tokens[rbrace + 2], "#") == 0) {
        mode = POST_INDEXED;
    } else { 
        // Malformed address, error
        fprintf(stderr, "Malformed address\n");
        exit(1);
    }

    if (mode == UNSIGNED_OFFSET && rbrace - lbrace == 2) {
        mode = ZERO_OFFSET;
    }

    return mode;
}

ParsedAddress parse_address(char** tokens, int token_count, SymbolTable symbol_table, uint32_t current_address) {
    AddressingMode mode;
    uint32_t sdt_ll_reg_rt = 0;
    uint32_t sdt_reg_xn = 0;
    uint32_t sdt_reg_xm = 0;
    bool sdt_ll_sf = false;
    uint32_t sdt_imm12 = 0;
    int32_t sdt_simm9 = 0;
    uint32_t ll_simm19 = 0; // Has to be unsigned due to specific implementation of symbol_table_get()

    int lbrace = -1;
    int rbrace = -1;
    int hashtag = -1;

    for (int i = 0; i < token_count; i++) {
        if (strcmp(tokens[i], "[") == 0) {
            lbrace = i;
        }
        if (strcmp(tokens[i], "]") == 0) {
            rbrace = i;
        }
        if (strcmp(tokens[i], "#") == 0) {
            hashtag = i;
        }
    }

    assert(hashtag == -1 || (lbrace < hashtag && hashtag < rbrace));

    mode = get_addressing_mode(tokens, token_count, lbrace, rbrace, hashtag);

    // Parsing address for relevant members 
    sdt_ll_reg_rt = (uint32_t)atoi(tokens[1]);
    sdt_ll_sf = (tokens[1][0] == 'x'); // Checks if rt is an X register
    if (is_sdt(mode)) { 
        assert(lbrace != -1);
        assert(rbrace != -1);
        assert(lbrace < rbrace);

        sdt_reg_xn = (uint32_t)atoi(tokens[lbrace + 1]);
        
        // Populating addressing mode-specific fields
        uint32_t imm = atoi(tokens[hashtag + 1]);
        switch (mode) {
            case ZERO_OFFSET:
                break;
            case UNSIGNED_OFFSET:
                (sdt_ll_sf) ? (sdt_imm12 = (uint32_t)(imm / 8)) : (sdt_imm12 = (uint32_t)(imm / 4));
                break;
            case PRE_INDEXED:
            case POST_INDEXED:
                sdt_simm9 = atoi(tokens[hashtag + 1]);
                break;
            case REGISTER_OFFSET:
                sdt_reg_xm = (uint32_t)atoi(tokens[lbrace + 3]);
                break;
            default:
                // Invalid addressing mode
                fprintf(stderr, "Addressing mode %s is invalid\n", STRINGIFY(mode));
                exit(1);
        }
    } else { // ldr rt, <literal>
        char *literal = tokens[3];
        if (symbol_table_get(symbol_table, literal, &ll_simm19)) {
            // Not a label, so parse as a literal int
            ll_simm19 = (uint32_t)atoi(literal);
        }
    }
    
    ParsedAddress address = {
        .mode = mode,
        .sdt_ll_reg_rt = sdt_ll_reg_rt,
        .sdt_reg_xn = sdt_reg_xn,
        .sdt_reg_xm = sdt_reg_xm,
        .sdt_ll_sf = sdt_ll_sf,
        .sdt_imm12 = sdt_imm12,
        .sdt_simm9 = sdt_simm9,
        .ll_simm19 = (int32_t)ll_simm19, // Need this cast as symbol_table_get modifies to unsigned
        // Trailing comma included for future changes' git history
    };
    return address;
}