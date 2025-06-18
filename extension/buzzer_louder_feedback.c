#include <stdio.h>
#include <stdlib.h>
#include <pigpiod_if2.h>
#include <unistd.h>
#include <signal.h> 

#include "constants.h"

// Global variable for pigpio connection, for cleanup
int pi;

// Cleanup function for Ctrl+C
void cleanup(int signum) {
    printf("\nSignal received. Cleaning up and stopping buzzer...\n");
    if (pi >= 0) {
        set_PWM_dutycycle(pi, BUZZER_PIN, 0); // Ensure buzzer is off
        pigpio_stop(pi);
    }
    exit(0);
}

int main(void) {
    signal(SIGINT, cleanup);
    pi = pigpio_start(NULL, NULL);
    if (pi < 0) {
        fprintf(stderr, "Failed to connect to pigpiod daemon.\n");
        return 1;
    }

    set_PWM_dutycycle(pi, BUZZER_PIN, 0);
    printf("Final Feedback System Running on GPIO %d.\n", BUZZER_PIN);
    printf("Reading from %s. Press Ctrl+C to stop.\n", INPUT_FILENAME);

    while (1) {
        FILE *file_ptr = fopen(INPUT_FILENAME, "r");
        float distance = -1.0;

        if (file_ptr == NULL) {
            usleep(50000); // Check file faster: every 50ms
            continue;
        }

        char buffer[32];
        if (fgets(buffer, sizeof(buffer), file_ptr) != NULL) {
            distance = atof(buffer);
        }
        fclose(file_ptr);

        if (distance > 0 && distance < 100) {
            int beep_frequency_hz = 2500; // A higher, more urgent pitch
            int beep_duration_ms = 50;    // A shorter, faster "tick"
            int silent_delay_ms;

            if (distance < 6) {
                // DANGER ZONE: Constant tone, no beeping.
                printf("!!! DANGER - Object at %.1f cm !!! Constant Tone\n", distance);
                set_PWM_frequency(pi, BUZZER_PIN, beep_frequency_hz);
                set_PWM_dutycycle(pi, BUZZER_PIN, 128);
                usleep(50000); // Small delay to prevent this loop from maxing out the CPU
                continue; // Skip the rest of the loop and read the file again
            }
            
            if (distance < 15) {
                // URGENT ZONE (< 15cm): Very aggressive curve.
                silent_delay_ms = (int)(distance * 4);
            } else if (distance < 40) {
                // WARNING ZONE (15-40cm): Steeper curve.
                silent_delay_ms = (int)(distance * 6);
            } else {
                // AWARENESS ZONE (40-100cm): Less aggressive initial curve.
                silent_delay_ms = (int)(distance * 9);
            }

            if (silent_delay_ms < 10) silent_delay_ms = 10;

            printf("Object at %.1f cm. Beep rhythm: %dms ON, %dms OFF.\n",
                   distance, beep_duration_ms, silent_delay_ms);

            set_PWM_frequency(pi, BUZZER_PIN, beep_frequency_hz);
            set_PWM_dutycycle(pi, BUZZER_PIN, 128); // Beep ON
            usleep(beep_duration_ms * 1000);

            set_PWM_dutycycle(pi, BUZZER_PIN, 0); // Beep OFF
            
            usleep(silent_delay_ms * 1000);

        } else {
            // If no object is close, turn buzzer off and check again quickly.
            set_PWM_dutycycle(pi, BUZZER_PIN, 0);
            usleep(50000); // Check file every 50ms
        }
    }
    
    return 0;
}