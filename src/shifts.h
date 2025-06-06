#ifndef SHIFTS_H
#define SHIFTS_H

#include <stdint.h>
#include <stdbool.h> // For bool type
#include "constants.h"

// Enum for shift types
typedef enum {
    SHIFT_LSL, // Logical Shift Left
    SHIFT_LSR, // Logical Shift Right
    SHIFT_ASR, // Arithmetic Shift Right
    SHIFT_ROR  // Rotate Right
} ShiftType;


//Performs a Logical Shift Left (LSL) operation.
uint64_t perform_lsl(uint64_t value, uint8_t shift_amount, bool is_64bit);

//Performs a Logical Shift Right (LSR) operation.
uint64_t perform_lsr(uint64_t value, uint8_t shift_amount, bool is_64bit);

//Performs an Arithmetic Shift Right (ASR) operation.
uint64_t perform_asr(uint64_t value, uint8_t shift_amount, bool is_64bit);

//Performs a Rotate Right (ROR) operation.
uint64_t perform_ror(uint64_t value, uint8_t shift_amount, bool is_64bit);

// Generic function to perform a shift based on ShiftType.
uint64_t execute_shift(uint64_t value, uint8_t shift_amount, ShiftType type, bool is_64bit);


#endif