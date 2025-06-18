#include <stdio.h>
#include <stdlib.h> 
#include <pigpiod_if2.h>
#include <unistd.h>
#include <signal.h> 

#include "constants.h"
#include "buzzer.h"

// Global variable for pigpio connection, for cleanup
int pi;

int main(void) {
    signal(SIGINT, cleanup);
    pi = pigpio_start(NULL, NULL);
    if (pi < 0) {
        fprintf(stderr, "Failed to connect to pigpiod daemon.\n");
        return 1;
    }

    set_PWM_dutycycle(pi, BUZZER_PIN, 0);
    printf("Dynamic Pitch with Custom Rhythm Feedback Running on GPIO %d.\n", BUZZER_PIN);
    printf("Reading from %s. Press Ctrl+C to stop.\n", INPUT_FILENAME);

    while (1) {
        FILE *file_ptr = fopen(INPUT_FILENAME, "r");
        float distance = -1.0;

        if (file_ptr == NULL) {
            usleep(SLEEP_TIME);
            continue;
        }

        char buffer[32];
        if (fgets(buffer, sizeof(buffer), file_ptr) != NULL) {
            distance = atof(buffer);
        }
        fclose(file_ptr);

        // DYNAMIC PITCH AND RHYTHM
        if (distance > MIN_DISTANCE_CM && distance < MAX_DISTANCE_CM) {
            int beep_duration_ms = 50;    // A short and fast "tick"
            int silent_delay_ms;
            int beep_frequency_hz;

            // Map distance [35cm -> 5cm] to frequency [800Hz -> 3000Hz]
            // A lower pitch for far away, a higher pitch for very close
            beep_frequency_hz = (int)(3500 - (distance * 70));
            // Clamp the frequency to a sensible audible range
            if (beep_frequency_hz < 800) beep_frequency_hz = 800;
            if (beep_frequency_hz > 3000) beep_frequency_hz = 3000;

            // DANGER ZONE : Constant High-Pitched Tone
            if (distance < 5) {
                printf("!!! DANGER - Object at %.1f cm !!! Constant Tone at %d Hz\n", distance, beep_frequency_hz);
                set_PWM_frequency(pi, BUZZER_PIN, beep_frequency_hz);
                set_PWM_dutycycle(pi, BUZZER_PIN, 128);
                usleep(50000); // Small delay to prevent this loop from maxing out the CPU
                continue; // Skip the beeping logic and re-check distance
            }
            
            if (distance < 10) {
                // URGENT ZONE
                silent_delay_ms = (int)(distance * 4);
            } else if (distance < 20) {
                // WARNING ZONE
                silent_delay_ms = (int)(distance * 6);
            } else {
                // AWARENESS ZONE
                silent_delay_ms = (int)(distance * 9);
            }

            if (silent_delay_ms < 10) silent_delay_ms = 10;

            printf("Object at %.1f cm. Freq: %dHz, Rhythm: %dms ON, %dms OFF.\n",
                   distance, beep_frequency_hz, beep_duration_ms, silent_delay_ms);

            // Use the NEW frequency but the OLD rhythm logic
            set_PWM_frequency(pi, BUZZER_PIN, beep_frequency_hz);
            set_PWM_dutycycle(pi, BUZZER_PIN, 128); // Beep ON
            usleep(beep_duration_ms * 1000);

            set_PWM_dutycycle(pi, BUZZER_PIN, 0); // Beep OFF
            usleep(silent_delay_ms * 1000);

        } else {
            // If no object is close, turn buzzer off and check again quickly.
            set_PWM_dutycycle(pi, BUZZER_PIN, 0);
            usleep(SLEEP_TIME);
        }
    }
    
    return 0;
}
