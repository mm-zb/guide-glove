#ifndef SYMBOL_TABLE_H
#define SYMBOL_TABLE_H

#include <stdint.h>
#include <stdbool.h>

// Pointer to hide implementation details
typedef struct SymbolTable* SymbolTable;

SymbolTable symbol_table_create(void);
void symbol_table_add(SymbolTable table, const char* label, uint32_t address);
bool symbol_table_get(SymbolTable table, const char* label, uint32_t* address_out);
void symbol_table_free(SymbolTable table);

#endif