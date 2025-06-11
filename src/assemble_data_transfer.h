#ifndef ASSEMBLE_DATA_TRANSFER_H
#define ASSEMBLE_DATA_TRANSFER_H

#include <stdint.h>

#include "symbol_table.h"
#include "tokenizer.h"

// These are all fixed bits that can't be extracted from the assembly
// and are needed to be included in specific instructions
#define SDT_BASE (0x2E0UL << 22) 
// 1 0 1 1 1 0 0 0 0 0 
// Bits 22 - 31 (not including bits 22, 24, and 30 as set elsewhere) 
#define LL_BASE (0x3UL << 27)
// 0 0 0 1 1 0 0 0
// Bits 24 - 31 (not including bit 30 as set elsewhere)
#define SDT_LL_SIGN_FLAG (1 << 30)// Bit 30
#define SDT_U (1UL << 24)// Bit 24
#define SDT_L (1UL << 22)// Bit 22
#define SDT_I (1UL << 11)// Bit 11
#define SDT_INDEXED (1UL << 10)// Bit 10 - denotes an pre-indexed or post-indexed instruction
#define SDT_REG (0x81AUL << 10)
// 1 0 0 0 0 0 0 1 1 0 1 0
// Bits 10 - 21 (not including bits 16 - 20)

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
    bool sdt_ll_sf;
    uint32_t sdt_imm12; // Handle sf-based division in parse_address
    int32_t sdt_simm9;
    int32_t ll_simm19;
} ParsedAddress;

// Parse address to populate ParsedAddress struct
ParsedAddress parse_address(char** tokens, SymbolTable* symbol_table, uint32_t current_address);

// Highest level call
uint32_t assemble_ldr(char** tokens, SymbolTable* symbol_table, uint32_t current_address);
uint32_t assemble_str(char** tokens, SymbolTable* symbol_table, uint32_t current_address);
uint32_t assemble_directive(char** tokens, SymbolTable* symbol_table, uint32_t current_address);

// Helper
uint32_t assemble_loadstore(char** tokens, SymbolTable* symbol_table, uint32_t current_address, bool is_ldr);

#endif