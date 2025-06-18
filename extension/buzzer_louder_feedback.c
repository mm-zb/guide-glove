#include <stdio.h>
#include <stdlib.h>      // For atof()
#include <pigpiod_if2.h>
#include <unistd.h>
#include <signal.h>      // For handling Ctrl+C

#include "constants.h"

// Global variable for pigpio connection, for cleanup
int pi;

void cleanup(int signum) {
    printf("\nSignal received. Cleaning up and stopping buzzer...\n");
    if (pi >= 0) {
        set_PWM_dutycycle(pi, BUZZER_PIN, 0); // Ensure buzzer is off
        pigpio_stop(pi);
    }
    exit(0);
}

int main(void) {
    // Set up the cleanup function
    signal(SIGINT, cleanup);

    // Connect to pigpio daemon
    pi = pigpio_start(NULL, NULL);
    if (pi < 0) {
        fprintf(stderr, "Failed to connect to pigpiod daemon.\n");
        return 1;
    }

    // Ensure the buzzer is off at the start.
    set_PWM_dutycycle(pi, BUZZER_PIN, 0);

    printf("Urgent-Curve Beeping Buzzer feedback system running on GPIO %d.\n", BUZZER_PIN);
    printf("Reading from %s. Press Ctrl+C to stop.\n", INPUT_FILENAME);

    while (1) {
        FILE *file_ptr = fopen(INPUT_FILENAME, "r");
        float distance = -1.0;

        if (file_ptr == NULL) {
            usleep(200000);
            continue;
        }

        char buffer[32];
        if (fgets(buffer, sizeof(buffer), file_ptr) != NULL) {
            distance = atof(buffer);
        }
        fclose(file_ptr);

        if (distance > 0 && distance < 100) {
            int beep_frequency_hz = 2000; // A consistently high, clear alert tone
            int beep_duration_ms = 60;   // A very short, sharp "tick"
            int silent_delay_ms;

            // function for non-linear feedback ---
            if (distance < 15) {
                // URGENT ZONE (< 15cm): Very aggressive curve.
                // Delay drops from ~45ms to 6ms. Sounds almost constant at the end.
                silent_delay_ms = (int)(distance * 3);
            } else if (distance < 40) {
                // WARNING ZONE (15-40cm): Steeper curve.
                // Delay drops from ~200ms to ~75ms.
                silent_delay_ms = (int)(distance * 5);
            } else {
                // AWARENESS ZONE (40-100cm): Less aggressive initial curve.
                // Delay drops from ~800ms to ~320ms.
                silent_delay_ms = (int)(distance * 8);
            }

            // Set a floor for the delay to ensure there's always a tiny break
            if (silent_delay_ms < 5) silent_delay_ms = 5;

            printf("Object at %.1f cm. Beep rhythm: %dms ON, %dms OFF.\n",
                   distance, beep_duration_ms, silent_delay_ms);

            // Turn the loud AC tone ON
            set_PWM_frequency(pi, BUZZER_PIN, beep_frequency_hz);
            set_PWM_dutycycle(pi, BUZZER_PIN, 128);

            // Wait for the beep duration
            usleep(beep_duration_ms * 1000);

            // Turn the tone OFF
            set_PWM_dutycycle(pi, BUZZER_PIN, 0);
            
            // Wait for the calculated silent period
            usleep(silent_delay_ms * 1000);

        } else {
            usleep(200000);
        }
    }
    
    return 0;
}