#ifndef SHIFTS_H
#define SHIFTS_H

#include <stdint.h>
#include <stdbool.h> // For bool type

// Enum for shift types, mirroring what's likely in DecodedInstruction.dp_reg_shift_type
// or you can directly use the numeric values if preferred.
// This is optional but can make the shift function interface clearer.
typedef enum {
    SHIFT_LSL = 0, // Logical Shift Left
    SHIFT_LSR = 1, // Logical Shift Right
    SHIFT_ASR = 2, // Arithmetic Shift Right
    SHIFT_ROR = 3  // Rotate Right
} ShiftType;


/**
 * @brief Performs a Logical Shift Left (LSL) operation.
 *
 * @param value The value to be shifted.
 * @param shift_amount The number of bits to shift.
 * @param is_64bit True if operating on a 64-bit value, false for 32-bit.
 *                 If false, the operation is performed on the lower 32 bits,
 *                 and the result is zero-extended to 64 bits.
 * @param carry_out Pointer to a boolean to store the carry out bit. Can be NULL if carry is not needed.
 * @return The result of the LSL operation.
 */
uint64_t perform_lsl(uint64_t value, uint8_t shift_amount, bool is_64bit, bool* carry_out);

/**
 * @brief Performs a Logical Shift Right (LSR) operation.
 *
 * @param value The value to be shifted.
 * @param shift_amount The number of bits to shift.
 * @param is_64bit True if operating on a 64-bit value, false for 32-bit.
 *                 If false, the operation is performed on the lower 32 bits,
 *                 and the result is zero-extended to 64 bits.
 * @param carry_out Pointer to a boolean to store the carry out bit. Can be NULL if carry is not needed.
 * @return The result of the LSR operation.
 */
uint64_t perform_lsr(uint64_t value, uint8_t shift_amount, bool is_64bit, bool* carry_out);

/**
 * @brief Performs an Arithmetic Shift Right (ASR) operation.
 *
 * Sign bit is replicated during the shift.
 *
 * @param value The value to be shifted.
 * @param shift_amount The number of bits to shift.
 * @param is_64bit True if operating on a 64-bit value, false for 32-bit.
 *                 If false, the operation is performed on the lower 32 bits
 *                 (sign extended from bit 31), and the result is sign-extended to 64 bits.
 * @param carry_out Pointer to a boolean to store the carry out bit. Can be NULL if carry is not needed.
 * @return The result of the ASR operation.
 */
uint64_t perform_asr(uint64_t value, uint8_t shift_amount, bool is_64bit, bool* carry_out);

/**
 * @brief Performs a Rotate Right (ROR) operation.
 *
 * Bits shifted out from the right are inserted on the left.
 *
 * @param value The value to be rotated.
 * @param shift_amount The number of bits to rotate.
 * @param is_64bit True if operating on a 64-bit value, false for 32-bit.
 *                 If false, the rotation is performed on the lower 32 bits.
 * @param carry_out Pointer to a boolean to store the carry out bit (which is the last bit rotated out).
 *                  Can be NULL if carry is not needed.
 * @return The result of the ROR operation (zero-extended to 64 bits if is_64bit is false).
 */
uint64_t perform_ror(uint64_t value, uint8_t shift_amount, bool is_64bit, bool* carry_out);

/**
 * @brief Generic function to perform a shift based on ShiftType.
 *        This can be a convenient wrapper around the specific shift functions.
 *
 * @param value The value to be shifted/rotated.
 * @param shift_amount The number of bits to shift/rotate.
 * @param type The type of shift/rotate operation (LSL, LSR, ASR, ROR).
 * @param is_64bit True if operating on a 64-bit value, false for 32-bit.
 * @param carry_out Pointer to a boolean to store the carry out from the shift.
 *                  This will be updated by the shift operation. Can be NULL.
 * @return The result of the shift/rotate operation.
 */
uint64_t execute_shift(uint64_t value, uint8_t shift_amount, ShiftType type, bool is_64bit, bool* carry_out);


#endif // SHIFTS_H