#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <inttypes.h>
#include "arm_state.h"

int main() {
    ARMState test_state;

    memset(&test_state, 0xFF, sizeof(ARMState));
    
    // Set PC to a distinct non-zero value
    test_state.pc = 0x123456789ABCDEF0ULL; 
    
    // Set PSTATE flags to opposite of expected initial state
    test_state.pstate.N = true;
    test_state.pstate.Z = false; 
    test_state.pstate.C = true;
    test_state.pstate.V = true;
    
    // Set a few arbitrary register values to non-zero
    test_state.registers[0] = 0xDEADBEEF;
    test_state.registers[5] = 0xCAFEBABE;
    test_state.registers[30] = 0xAAAAAAAA;
    
    // Set a few memory bytes to non-zero
    test_state.memory[0] = 0x11;
    test_state.memory[100] = 0x22;
    // For 2MB memory, max index is 2*1024*1024 - 1
    test_state.memory[2 * 1024 * 1024 - 1] = 0x33;


    printf("--- Running initialize_arm_state test ---\n");

    initialize_arm_state(&test_state);

    // 1. Verify General Purpose Registers (X0-X30)
    printf("Verifying registers (X0-X30)... ");
    for (int i = 0; i < 31; i++) {
        if (test_state.registers[i] != 0) {
            // CHANGE: %016llX -> %016"PRIx64"
            printf("\nFAIL: Register X%d is 0x%016"PRIx64", expected 0.\n", i, test_state.registers[i]);
            return 1;
        }
    }
    printf("All OK.\n");

    // 2. Verify Program Counter (PC)
    printf("Verifying PC... ");
    if (test_state.pc != 0) {
        // CHANGE: %016llX -> %016"PRIx64"
        printf("\nFAIL: PC is 0x%016"PRIx64", expected 0.\n", test_state.pc);
        return 1;
    }
    // CHANGE: %016llX -> %016"PRIx64"
    printf("OK (0x%016"PRIx64").\n", test_state.pc);

    // 3. Verify PSTATE flags
    printf("Verifying PSTATE flags... ");
    if (test_state.pstate.N != false ||
        test_state.pstate.Z != true || // Z should be true as per problem statement
        test_state.pstate.C != false ||
        test_state.pstate.V != false) {
        printf("\nFAIL: PSTATE incorrect. Expected N=F, Z=T, C=F, V=F.\n");
        printf("  Actual: N=%d, Z=%d, C=%d, V=%d\n",
               (int)test_state.pstate.N, (int)test_state.pstate.Z,
               (int)test_state.pstate.C, (int)test_state.pstate.V);
        return 1;
    }
    printf("OK (N: %d, Z: %d, C: %d, V: %d).\n",
           (int)test_state.pstate.N, (int)test_state.pstate.Z,
           (int)test_state.pstate.C, (int)test_state.pstate.V);

    // 4. Verify Memory (check a few specific spots for practical purposes)
    // Full memory contents are implicitly zeroed by memset, but checking a few spots is good.
    printf("Verifying memory (first and last 16 bytes)... ");
    for (size_t i = 0; i < 16; i++) {
        if (test_state.memory[i] != 0) {
            printf("\nFAIL: Memory byte at address %zu is 0x%02X, expected 0.\n", i, test_state.memory[i]);
            return 1;
        }
    }

    for (size_t i = MEMORY_SIZE - 16; i < MEMORY_SIZE; i++) {
        if (test_state.memory[i] != 0) {
            printf("\nFAIL: Memory byte at address %zu is 0x%02X, expected 0.\n", i, test_state.memory[i]);
            return 1;
        }
    }
    printf("All OK.\n");
    printf("  (Full memory contents are assumed to be zero based on memset calls in initialize_arm_state)\n");

    printf("\nAll tests passed successfully for initialize_arm_state!\n");
    return 0;
}