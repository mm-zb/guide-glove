#include "dp_executor.h"

static uint64_t get_register_value(ARMState* state, uint8_t reg_idx, bool is_64bit) {
    if (reg_idx == 31) return 0;  // ZR is register 31
    if (is_64bit) {
        return state->registers[reg_idx];
    } else {
        return (uint32_t)state->registers[reg_idx];  // Read lower 32 bits
    }
}

static void set_register_value(ARMState* state, uint8_t reg_idx, uint64_t value, bool is_64bit) {
    if (reg_idx == 31) return;  // Writes to ZR are ignored
    if (is_64bit) {
        state->registers[reg_idx] = value;
    } else {
        // Writing to Wn clears upper 32 bits of Xn
        state->registers[reg_idx] = (uint64_t)(uint32_t)value;
    }
}

// --- Forward declarations for static functions ---
static void execute_dp_imm_instruction(ARMState* state, DecodedInstruction* instr);
static void execute_dp_reg_instruction(ARMState* state, DecodedInstruction* instr);
static void update_pstate_flags(ARMState* state, uint64_t result, uint64_t op1, uint64_t op2,
                                DecodedInstruction* instr);

void execute_dp_instruction(ARMState* state, DecodedInstruction* instr) {
    switch (instr->type) {
        case DP_IMM:
            execute_dp_imm_instruction(state, instr);
            break;
        case DP_REG:
            execute_dp_reg_instruction(state, instr);
            break;
        case SDT:
        case LL:
        case BRANCH:
        case HALT:
        case UNKNOWN:
        default:
            fprintf(stderr,
                    "Error: execute_dp_instruction called with unhandled instruction type %d\n",
                    instr->type);
            break;
    }
}

static void execute_dp_imm_instruction(ARMState* state, DecodedInstruction* instr) {
    bool sf = instr->sf;  // 0 for 32-bit (W reg), 1 for 64-bit (X reg)
    uint64_t operand1_val;
    uint64_t immediate_val;
    uint64_t result;

    switch (instr->dp_imm_opi) {
        // opi == 010 (binary) or 0x2 (hex) => Arithmetic operation
        case 0x02: {
            immediate_val = (uint64_t)instr->dp_imm_imm12;  // imm12 is unsigned

            if (instr->dp_imm_sh == 1) {  // sh field from DecodedInstruction
                immediate_val <<= 12;
            }

            operand1_val = get_register_value(state, instr->dp_rn, sf);

            // dp_opc: 00=ADD, 01=ADDS, 10=SUB, 11=SUBS
            switch (instr->dp_opc) {
                case 0x00:  // ADD
                case 0x01:  // ADDS
                    result = operand1_val + immediate_val;
                    set_register_value(state, instr->dp_rd, result, sf);
                    if (instr->dp_opc == 0x01)  // if ADDS update PSTATE
                        update_pstate_flags(state, result, operand1_val, immediate_val, instr);
                    break;

                case 0x02:  // SUB
                case 0x03:  // SUBS (dp_opc 11 binary is 0x3)
                    result = operand1_val - immediate_val;
                    set_register_value(state, instr->dp_rd, result, sf);
                    if (instr->dp_opc == 0x03) {  // if SUBS update PSTATE
                        update_pstate_flags(state, result, operand1_val, immediate_val, instr);
                    }
                    break;
            }
            break;
        }
        // opi == 101 (binary) or 0x5 (hex) => Wide move operation (MOVN, MOVZ, MOVK)
        case 0x05: {
            uint16_t imm16 = instr->dp_imm_imm16;
            uint8_t hw_shift = instr->dp_imm_hw * 16;  // hw is 0,1,2,3 for 64-bit; 0,1 for 32-bit
            uint64_t operand_to_move = (uint64_t)imm16 << hw_shift;

            // dp_opc for wide moves: 00=MOVN, 10=MOVZ, 11=MOVK
            switch (instr->dp_opc) {
                case 0x00:  // MOVN
                    result = sf ? ~operand_to_move : (uint32_t)(~operand_to_move);
                    // For 32-bit MOVN, result should be ~imm shifted into lower 32-bits, upper
                    // 32-bits zero. set_register_value handles the Wn clearing upper bits behavior.
                    break;
                case 0x02:  // MOVZ
                    result = operand_to_move;
                    // For 32-bit MOVZ, result is imm shifted into lower 32-bits, upper 32-bits
                    // zero.
                    break;
                case 0x03: {  // MOVK
                    uint64_t current_rd_val = get_register_value(state, instr->dp_rd, sf);
                    uint64_t mask_16bit_at_pos = (sf ? 0xFFFFULL : 0xFFFFU) << hw_shift;
                    result = (current_rd_val & ~mask_16bit_at_pos) | operand_to_move;
                    break;
                }
            }
            set_register_value(state, instr->dp_rd, result, sf);
            break;
        }
    }
}

// --- DP Register Instruction Execution ---
static void execute_dp_reg_instruction(ARMState* state, DecodedInstruction* instr) {
    bool sf = instr->sf;  // 0 for 32-bit (W reg), 1 for 64-bit (X reg)
    uint64_t val_rn, val_rm, val_ra;
    uint64_t operand2;
    uint64_t result;
    // bool carry_from_shift = false; // To capture carry from shift operations for PSTATE.C

    // Get operand values from registers
    val_rn = get_register_value(state, instr->dp_rn, sf);
    val_rm = get_register_value(state, instr->dp_reg_rm, sf);

    // Calculate Operand2: Apply shift to Rm
    operand2 =
        execute_shift(val_rm, instr->dp_reg_shift_amount, (ShiftType)instr->dp_reg_shift_type, sf);

    if (instr->dp_reg_M == 0) {
        // --- Arithmetic or Logical Operation (M=0) ---
        // For logical operations, operand2 might be negated if N=1
        if (instr->dp_reg_N == 1 &&
            (instr->dp_reg_opr >> 3) == 0) {  // N bit is part of opr for logical
            operand2 = sf ? ~operand2 : (uint32_t)(~operand2);
        }

        // uint8_t logical_op_selector = (instr->dp_reg_opr >> 1) & 0x3; // Extracts opc field (bits
        // 2-1 of opr) bool S_flag = (instr->dp_opc & 0x1); // Bit 0 of dp_opc usually indicates if
        // it sets flags (e.g. ADDS vs ADD)

        if (!((instr->dp_reg_opr >> 3) & 0x1)) {  // If bit 3 of opr is 0, it's logical

            bool set_flags = (instr->dp_opc == 3);  // set flags when opc == 11 (ANDS, BICS)

            switch (instr->dp_opc) {
                case 0x00:  // AND, BIC (TST if S=1 and Rd=ZR, ANDS if S=1)
                case 0x03:  // ANDS, BICS
                    // N bit (instr->dp_reg_N) determines AND vs BIC
                    result = val_rn & operand2;
                    break;
                case 0x01:  // ORR, ORN
                    result = val_rn | operand2;
                    break;
                case 0x02:  // EOR, EON
                    result = val_rn ^ operand2;
                    break;
            }
            if (set_flags ||
                instr->dp_rd == 31) {  // If S bit is set, or if Rd is ZR (like TST, CMP)
                update_pstate_flags(state, result, val_rn, operand2, instr);
            }
            set_register_value(state, instr->dp_rd, result, sf);

        } else {  // Arithmetic: ADD, ADDS, SUB, SUBS (M=0, opr[3]=1)
            bool is_subtract = (instr->dp_opc & 0x02);

            if (is_subtract) {  // SUB or SUBS
                result = val_rn - operand2;
            } else {  // ADD or ADDS
                result = val_rn + operand2;
            }
            if (!sf) result = (uint32_t)result;  // For 32-bit, truncate to lower 32 bits
            
            set_register_value(state, instr->dp_rd, result, sf);

            bool set_flags = (instr->dp_opc & 0x01);  // S flag (instr bit 29)
            if (set_flags || instr->dp_rd == 31) {    // If S bit is set or Rd is ZR (CMP, CMN)
                update_pstate_flags(state, result, val_rn, operand2, instr);
            }
        }

    } else if (instr->dp_reg_M == 1) {
        // --- Multiply Operation (M=1) ---
        // dp_reg_x: 0 for MADD, 1 for MSUB

        val_ra = get_register_value(state, instr->dp_reg_ra, sf);
        uint64_t product_operand2 = val_rm;  // Use unshifted val_rm for multiply

        uint64_t product;
        if (sf) {  // 64-bit multiply
            product = val_rn * product_operand2;
        } else {  // 32-bit multiply
            product = (uint64_t)((uint32_t)val_rn * (uint32_t)product_operand2);
        }

        if (instr->dp_reg_x == 0) {  // MADD
            result = val_ra + product;
        } else {  // MSUB
            result = val_ra - product;
        }
        set_register_value(state, instr->dp_rd, result, sf);
        // Multiply instructions (MADD, MSUB) do not set PSTATE flags.
    }
}

static void update_pstate_flags(ARMState* state, uint64_t result, uint64_t op1, uint64_t op2,
                                DecodedInstruction* instr) {
    // Update N flag (sign bit of result)
    state->pstate.N = (result >> (instr->sf ? 63 : 31)) & 1;

    // Update Z flag (result is zero)
    state->pstate.Z = (result == 0);

    // Determine if the instruction is arithmetic (IMM: opi == 010 REG: M == 0 and opr[3] == 1)
    bool is_arithmetic =
        (instr->type == DP_IMM && instr->dp_imm_opi == 2) ||
        (instr->type == DP_REG && instr->dp_reg_M == 0 && (instr->dp_reg_opr >> 3) == 1);

    // Update C flag (1 if carry/borrow)
    if (is_arithmetic) {
        // ADDS: C is set if there was a carry from the addition
        if (instr->dp_opc == 0x1) state->pstate.C = (result < op1);
        // SUBS: C is set if there was NO borrow
        if (instr->dp_opc == 0x3) state->pstate.C = (op1 >= op2);
    } else if (instr->type == DP_REG) {
        state->pstate.C = 0;
    }

    // Update V flag (1 if signed overflow/underflow)
    int64_t sop1 = (int64_t)op1;
    int64_t sop2 = (int64_t)op2;
    int64_t sresult = (int64_t)result;
    if (is_arithmetic) {
        if (instr->dp_opc == 0x1)  // ADDS
            state->pstate.V =
                (sop1 > 0 && sop2 > 0 && sresult < 0) || (sop1 < 0 && sop2 < 0 && sresult > 0);
        if (instr->dp_opc == 0x3)  // SUBS
            state->pstate.V =
                (sop1 > 0 && sop2 < 0 && sresult < 0) || (sop1 < 0 && sop2 > 0 && sresult > 0);
    } else if (instr->type == DP_REG) {
        state->pstate.V = 0;  // No overflow for logical or multiply
    }
}