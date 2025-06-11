#ifndef ASSEMBLE_DATA_TRANSFER_H
#define ASSEMBLE_DATA_TRANSFER_H

#include "symbol_table.h"
#include "tokenizer.h"

#include <stdint.h>

// Adding the below in the correct combination, along with the other relevant
// components will give the final binary instruction
#define SDT_BASE (0x2E0UL << 22) 
// 1 0 1 1 1 0 0 0 0 0 
// Bits 22 - 31 (not including bits 22, 24, and 30 as set elsewhere) 
#define LL_BASE (0x3UL << 27)
// 0 0 0 1 1 0 0 0
// Bits 24 - 31 (not including bit 30 as set elsewhere)
#define SDT_LL_SIGN_FLAG (1 << 30)// Bit 30
#define SDT_U (1 << 24)// Bit 24
#define SDT_L (1 << 22)// Bit 22

typedef enum {
    ZERO_OFFSET,
    UNSIGNED_OFFSET,
    PRE_INDEXED,
    POST_INDEXED,
    REGISTER_OFFSET,
    LOAD_LITERAL
} AddressingMode;

typedef struct {
    AddressingMode mode;
    uint32_t sdt_ll_reg_rt;
    uint32_t sdt_reg_xn;
    uint32_t sdt_reg_xm;
    int32_t sdt_imm12; 
    uint32_t sdt_simm9;
    uint32_t ll_simm19;
    uint32_t sdt_offset;   
} ParsedAddress;

// Parse address to populate ParsedAddress struct
ParsedAddress parse_address(char** tokens);

// Highest level call
uint32_t assemble_ldr(char** tokens, SymbolTable* symbol_table, uint32_t current_address);
uint32_t assemble_str(char** tokens, SymbolTable* symbol_table, uint32_t current_address);
uint32_t assemble_directive(char** tokens, SymbolTable* symbol_table, uint32_t current_address);

#endif