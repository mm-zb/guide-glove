#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "symbol_table.h"

// Linked List Node
typedef struct Node {
    char* label;
    uint32_t address;
    struct Node* next;
} Node;

// SymbolTable struct holds the head of the list
struct SymbolTable {
    Node* head;
};

SymbolTable symbol_table_create(void) {
    SymbolTable table = malloc(sizeof(struct SymbolTable));
    if (table == NULL) {
        return NULL;
    }
    table->head = NULL;
    return table;
}

void symbol_table_add(SymbolTable table, const char* label, uint32_t address) {
    Node* new_node = malloc(sizeof(Node));
    if (new_node == NULL) {
        perror("Failed to allocate node");
        return;
    }

    new_node->label = strdup(label); // strdup allocates and copies
    if (new_node->label == NULL) {
        perror("Failed to duplicate label");
        free(new_node);
        return;
    }

    new_node->address = address;
    new_node->next = table->head;
    table->head = new_node;
}

bool symbol_table_get(SymbolTable table, const char* label, uint32_t* address_out) {
    for (Node* current = table->head; current != NULL; current = current->next) {
        if (strcmp(current->label, label) == 0) {
            *address_out = current->address;
            return true;
        }
    }
    return false;
}

void symbol_table_free(SymbolTable table) {
    if (table == NULL) {
        return;
    }

    Node* current = table->head;
    while (current != NULL) {
        Node* next = current->next;
        free(current->label);
        free(current);
        current = next;
    }
    free(table);
}