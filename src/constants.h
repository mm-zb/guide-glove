#ifndef CONSTANTS_H
#define CONSTANTS_H

#define MEMORY_SIZE (2 * 1024 * 1024) // 2MB
#define HALT_INSTRUCTION 0x8a000000
#define ADDRESS_REGISTER_XZR 0x1F
// Define MASK_32BIT as a 64-bit value with lower 32 bits set, for masking 64-bit variables to 32-bit effective width
#define MASK_32BIT ((uint64_t)0x00000000FFFFFFFFULL) 
#define MASK_64BIT ((uint64_t)0xFFFFFFFFFFFFFFFFULL)

typedef enum {
    UNSIGNED_IMMEDIATE,
    PRE_INDEXED,
    POST_INDEXED,
    REGISTER_OFFSET,
    LOAD_LITERAL,
} addressing_mode;

#endif