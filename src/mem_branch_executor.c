#include "mem_branch_executor.h"
// constants.h included implicitly through mem_branch_executor.h

#define GPIO_BASE 0x3f200000
#define GPFSEL2   (GPIO_BASE + 0x08)
#define GPSET0    (GPIO_BASE + 0x1C)
#define GPCLR0    (GPIO_BASE + 0x28)
#define GPIO_END 0x3f200030

// PC = PC + 4 * simm26
void execute_branch_unconditional(ARMState* state, int64_t simm26) {
    uint64_t next_pc;
    uint64_t curr_pc;
    int64_t offset; // Signed

    offset = simm26 << 2;
    curr_pc = state->pc;
    next_pc = curr_pc + offset;
    state->pc = next_pc;
}

// PC = Xn
void execute_branch_register(ARMState* state, uint8_t register_xn) {
    uint8_t xzr = ADDRESS_REGISTER_XZR;
    uint64_t next_pc;
    uint64_t target_address;

    if (register_xn == xzr) {
        // We are at the zero register, we do not need to handle this case
        // So abort early and the PC remains unchanged
        // This might need to set the PC to zero but I am unsure for now
        return;
    }

    target_address = state->registers[register_xn];
    next_pc = target_address;
    state->pc = next_pc;
}

// If cond then PC = PC + 4 * simm19
void execute_branch_cond(ARMState* state, int64_t simm19, uint8_t cond) {
    // const int EQ = 0x0;
    // const int NE = 0x1;
    // const int GE = 0xA;
    // const int LT = 0xB;
    // const int GT = 0xC;
    // const int LE = 0xD;
    // const int AL = 0xE;

    // Breaking out of the switch statement is the same as branching
    // Returning is the same as leaving the PC unchanged
    switch (cond) {
        case 0x0:
            if (state->pstate.Z) break;
            return;
        case 0x1:
            if (!state->pstate.Z) break;
            return;
        case 0xA:
            if (state->pstate.N == state->pstate.V) break;
            return;
        case 0xB:
            if (state->pstate.N != state->pstate.V) break;
            return;
        case 0xC:
            if (state->pstate.Z==0 && state->pstate.N == state->pstate.V) break;
            return;
        case 0xD:
            if (!(state->pstate.Z==0 && state->pstate.N == state->pstate.V)) break;
            return;
        case 0xE:
            break;
        default:
            // Invalid condition code
            // For now I will leave the PC unchanged
            return;
    }

    execute_branch_unconditional(state, simm19);
}

// Calculates address and moves data into memory, from register rt
void execute_str(ARMState* state, addressing_mode addr_mode, DecodedInstruction* instruction) {
    uint64_t address;
    uint8_t sf;

    uint8_t register_rt;
    uint64_t target_register;

    int bytes_stored;

    address = calculate_address(state, addr_mode, instruction);
    sf = instruction->sf;

    register_rt = instruction->sdt_ll_rt;
    target_register = state->registers[register_rt];

    if (address >= GPIO_BASE && address < GPIO_END) {
    // The Raspberry Pi spec requires GPIO access to be 32-bit wide.
        if (sf != 0) { // sf=0 for 32-bit (W registers)
            fprintf(stderr, "Warning: 64-bit STR to GPIO address 0x%08lx ignored.\n", address);
            return;
        }

        uint32_t value_to_store = (uint32_t)target_register;

        // Check which GPIO register is being accessed
        switch (address) {
            case GPFSEL2:
                printf("One GPIO pin from 20 to 29 has been configured\n");
                break;
            case GPSET0:
                // For turning the LED ON
                // Check if the bit for pin 21 is set
                if (value_to_store & (1 << 21)) {
                    printf("PIN ON\n");
                }
                break;
            case GPCLR0:
                // For turning the LED OFF
                if (value_to_store & (1 << 21)) {
                    printf("PIN OFF\n");
                }
                break;
        }
        // After logging the GPIO action we need to return
        // This prevents the emulator from writing to its main memory model
        return;
    }
    // Conditional write depending on sf
    else if (sf == 0) { // Store a 32-bit word
        bytes_stored = 4;
    } else { // Store a 64-bit doubleword
        bytes_stored = 8;
    }
    for (int i=0; i<bytes_stored; i++) {
        state->memory[address + i] = (target_register >> 8*i) & 0xFF;
    }
}

// Calculates address and moves data into register rt, from memory
void execute_ldr(ARMState* state, addressing_mode addr_mode, DecodedInstruction* instruction) {
    uint64_t address;
    uint8_t sf;
    int64_t simm19;

    uint8_t register_rt;
    uint64_t data_to_store = 0;

    int bytes_stored;

    sf = instruction->sf;

    if (addr_mode == LOAD_LITERAL) {
        simm19 = (int64_t)instruction->ll_simm19;
        address = get_address_load_literal(state, simm19);
    } else {
        address = calculate_address(state, addr_mode, instruction);
    }
    
    register_rt = instruction->sdt_ll_rt;

    // Conditional write depending on sf
    if (sf == 0) { // Store a 32-bit word
        bytes_stored = 4;
    } else { // Store a 64-bit doubleword
        bytes_stored = 8;
    }
    for (int i=0; i<bytes_stored; i++) {
        data_to_store |= (uint64_t)state->memory[address + i] << (8 * i);
    }
    state->registers[register_rt] = data_to_store;
}

// Calculates address with addressing mode
uint64_t calculate_address(ARMState* state, addressing_mode addr_mode, DecodedInstruction* instruction) {
    uint64_t address;
    uint16_t offset;
    uint8_t sf;
    uint8_t I;

    uint8_t register_xn;
    uint8_t register_xm;

    int64_t simm9;

    offset = instruction->sdt_imm12;
    register_xn = instruction->sdt_xn;
    sf = instruction->sf;

    switch (addr_mode) {
    case (UNSIGNED_IMMEDIATE): 
        address = get_address_unsigned_immediate(state, (uint64_t)offset, register_xn, sf);
        break;
    case (PRE_INDEXED):
    case (POST_INDEXED):
        simm9 = (int64_t)instruction->sdt_simm9;
        I = (uint8_t)instruction->sdt_I;
        // Takes the correct 9 bits for the offset and the index flag I
        address = get_address_indexed(state, simm9, register_xn, I);
        break;
    case (REGISTER_OFFSET):
        register_xm = (uint8_t)instruction->sdt_xm;
        address = get_address_register_offset(state, register_xm, register_xn);
        break;
    default:
        // Invalid addressing mode
        // Do nothing for now
        fprintf(stderr, "Invalid addressing mode!");
        return 0;
    }
    return address;
}