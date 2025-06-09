#include "shifts.h"

uint64_t perform_lsl(uint64_t value, uint8_t shift_amount, bool is_64bit) {
    if (!is_64bit) {
        // Ensure 32-bit operations only affect lower 32 bits of the 64-bit value
        value = value & MASK_32BIT;
    }

    uint64_t output = value << shift_amount;
    // For 32-bit LSL, if original value was masked, result will effectively be in lower 32 bits.
    // Upper bits of 'output' will be zero due to the nature of left shift on a masked value or from a 32-bit context.
    return output;
}

uint64_t perform_lsr(uint64_t value, uint8_t shift_amount, bool is_64bit){
    if (!is_64bit) {
        // Ensure 32-bit operations only affect lower 32 bits
        value = value & MASK_32BIT;
    }
    // Logical shift right fills vacated MSBs with zeros automatically in C for unsigned types
    uint64_t output = value >> shift_amount;
    return output;
}

uint64_t perform_asr(uint64_t value, uint8_t shift_amount, bool is_64bit){
    int sign_bit_position;
    bool is_negative = false;
    uint64_t sign_fill_mask = 0; // Mask to fill vacated bits if negative
 
    if (is_64bit) {
        sign_bit_position = 63;
        // If negative, create a mask of 1s for the bits that will be shifted in
        if ((value >> sign_bit_position) & 1) { // Check MSB of 64-bit value
            is_negative = true;
            if (shift_amount > 0 && shift_amount < 64) { // Avoid issues with shift by 0 or >= 64
            sign_fill_mask = MASK_64BIT << (64 - shift_amount);
            } else if (shift_amount >= 64) {
            sign_fill_mask = MASK_64BIT; // All bits become sign bit
            }
        }
    } else { 
        value &= MASK_32BIT; // Work with lower 32 bits
        sign_bit_position = 31;
        // If negative (MSB of the 32-bit value is 1), create a mask
        if ((value >> sign_bit_position) & 1) {
            is_negative = true;
            if (shift_amount > 0 && shift_amount < 32) {
                sign_fill_mask = MASK_32BIT << (32 - shift_amount);
                sign_fill_mask &= MASK_32BIT; // Ensure mask is also 32-bit
            } else if (shift_amount >= 32) {
                sign_fill_mask = MASK_32BIT; // All bits become sign bit (within 32-bit context)
            }
        }
    }
    
    // Perform the right shift
    uint64_t output = value >> shift_amount;

    // If the original number was negative, apply the sign fill mask
    if (is_negative) { // Re-check sign bit of original (relevant part of) value
         if (is_64bit && shift_amount >= 64) output = MASK_64BIT;
         else if (!is_64bit && shift_amount >= 32) output = MASK_32BIT;
         else output = output | sign_fill_mask;
    }
    
    if (!is_64bit) {
        output &= MASK_32BIT; // Ensure result for 32-bit op is confined to lower 32 bits
    }
    return output;
}

uint64_t perform_ror(uint64_t value, uint8_t shift_amount, bool is_64bit) {
    uint64_t rotated_bits;
    unsigned int bits = is_64bit ? 64 : 32;
    uint64_t current_mask = is_64bit ? MASK_64BIT : MASK_32BIT;

    if (!is_64bit) {
        value &= MASK_32BIT; // Operate on the relevant part of the value
    }

    shift_amount %= bits; // Effective shift amount for rotation

    if (shift_amount == 0) {
        return value; // No rotation needed
    }

    // Isolate bits that will be rotated from LSB to MSB
    rotated_bits = value & ((1ULL << shift_amount) - 1); // Creates a mask of 'shift_amount' ones
    
    // Shift these bits to their new MSB positions
    rotated_bits <<= (bits - shift_amount);

    // Perform the main right shift
    uint64_t output = value >> shift_amount;
    
    // Combine the shifted part with the rotated bits
    output |= rotated_bits;

    return output & current_mask; // Ensure result is within the defined bit width
}

uint64_t execute_shift(uint64_t value, uint8_t shift_amount, ShiftType type, bool is_64bit) {
    // This function dispatches to the appropriate specific shift operation.
    // Assumes ShiftType enum (SHIFT_LSL, etc.) is defined in shifts.h
    uint64_t output;
    switch (type) {
        case SHIFT_LSL: 
            output = perform_lsl(value, shift_amount, is_64bit);
            break;
        case SHIFT_LSR: 
            output = perform_lsr(value, shift_amount, is_64bit);
            break;
        case SHIFT_ASR: 
            output = perform_asr(value, shift_amount, is_64bit);
            break;
        case SHIFT_ROR: 
            output = perform_ror(value, shift_amount, is_64bit);
            break;
    }
    return is_64bit ? output : (uint32_t)output;
}