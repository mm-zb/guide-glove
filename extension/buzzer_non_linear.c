#include <stdio.h>
#include <stdlib.h>
#include <pigpiod_if2.h>
#include <unistd.h> 
#include <math.h>  

#include "constants.h"

// Local Configuration Constants for Feedbac-
#define MIN_DELAY_MS 80 
#define MAX_DELAY_MS 1000

#define BEEP_DURATION_US 50000 // How long each beep lasts in microseconds (50ms)
#define CURVE_POWER 2.5f       // The steepness of the curve. 2.0 is squared, 3.0 is cubed. 2.5 is a good middle ground


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

    printf("Non-Linear Buzzer feedback running. Reading from %s. Press Ctrl+C to stop.\n", INPUT_FILENAME);

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

        // --- Non-Linear Feedback Logic ---
        if (distance >= MIN_DISTANCE_CM && distance <= MAX_DISTANCE_CM) {
            
            // 1. Normalize the distance to a value between 0.0 and 1.0
            // Here, 0.0 is farthest (100cm) and 1.0 is closest (2cm)
            // We flip it so that "more" means "more urgent"
            float normalized_urgency = (MAX_DISTANCE_CM - distance) / (MAX_DISTANCE_CM - MIN_DISTANCE_CM);

            // 2. Apply the power curve to make the change non-linear
            // The pow() function is from math.h (pow(base, exponent))
            float power_curve = powf(normalized_urgency, CURVE_POWER);
            
            // 3. Map the curved value (0.0 to 1.0) to our delay range
            int delay_range = MAX_DELAY_MS - MIN_DELAY_MS;
            int delay_ms = MAX_DELAY_MS - (int)(power_curve * delay_range);

            printf("Object at %.1f cm. Beeping every %d ms (Urgency: %.2f).\n", distance, delay_ms, normalized_urgency);

            // Beep for a fixed duration
            gpio_write(pi, BUZZER_PIN, 1);
            usleep(BEEP_DURATION_US); 
            gpio_write(pi, BUZZER_PIN, 0);
            
            // Wait for the calculated non-linear delay
            usleep(delay_ms * 1000);

        } else {
            // If no object is close or the reading is invalid, be quiet.
            gpio_write(pi, BUZZER_PIN, 0);
            // Check the file less frequently when there's nothing to do
            usleep(SLEEP_TIME); 
        }
    }

    pigpio_stop(pi);
    return 0;
}
