#include <stdio.h>
#include <stdlib.h> 
#include <pigpiod_if2.h>
#include <unistd.h> 

#include "constants.h"

#define INPUT_FILENAME "distances.txt"

int main(void) {
    // Connect to local pigpiod daemon
    int pi = pigpio_start(NULL, NULL);
    if (pi < 0) {
        fprintf(stderr, "Failed to connect to pigpiod daemon\n");
        return 1;
    }

    // Configure the buzzer pin
    set_mode(pi, BUZZER_PIN, PI_OUTPUT);
    gpio_write(pi, BUZZER_PIN, 0); // Start with buzzer off

    printf("Buzzer feedback system running. Reading from %s. Press Ctrl+C to stop.\n", INPUT_FILENAME);

    while (1) {
        // Open the file in "read" mode ('r')
        FILE *file_ptr = fopen(INPUT_FILENAME, "r");
        float distance = -1.0;

        if (file_ptr == NULL) {
            // If the file doesn't exist yet, just wait and try again
            usleep(SLEEP_TIME);
            continue;
        }

        // Read the distance value from the file
        char buffer[32];
        if (fgets(buffer, sizeof(buffer), file_ptr) != NULL) {
            distance = atof(buffer);
        }
        fclose(file_ptr);

        if (distance > 0 && distance < 100) { // Only react if distance is < 100cm
            // The closer the object, the shorter the delay between beeps.
            // Map distance (100cm -> 2cm) to a delay (500ms -> 50ms)
            int delay_ms = (int)(distance * 4.5) + 50;
            if (delay_ms > 500) delay_ms = 500;

            printf("Object at %.1f cm. Beeping every %d ms.\n", distance, delay_ms);

            // Beep for 50ms
            gpio_write(pi, BUZZER_PIN, 1);
            usleep(SLEEP_TIME);
            gpio_write(pi, BUZZER_PIN, 0);
            
            // Wait for the calculated delay
            usleep(delay_ms * 1000);

        } else {
            // If no object is close, just be quiet and wait.
            gpio_write(pi, BUZZER_PIN, 0);
            usleep(SLEEP_TIME); // Check the file every 0.2s
        }
    }

    pigpio_stop(pi);
    return 0;
}
