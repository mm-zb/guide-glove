#include <stdio.h>
#include <stdlib.h>
#include <pigpiod_if2.h>
#include <unistd.h>
#include <signal.h>

#include "constants.h"
#include "buzzer.h"

int pi;


int main(void) {
    signal(SIGINT, cleanup);
    pi = pigpio_start(NULL, NULL);
    if (pi < 0) {
        fprintf(stderr, "Failed to connect to pigpiod daemon.\n");
        return 1;
    }

    // Setup pins
    set_PWM_dutycycle(pi, BUZZER_PIN, 0);     // Initialize buzzer off
    set_mode(pi, MOTOR_PIN, PI_OUTPUT);      // Set motor pin as an output
    gpio_write(pi, MOTOR_PIN, 0);            // Initialize motor off

    printf("Haptic & Audio Feedback System Running.\n");
    printf("Buzzer: GPIO %d, Motor: GPIO %d\n", BUZZER_PIN, MOTOR_PIN);
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

        // Dynamic pitch and rhythm
        if (distance > 0 && distance < 35) {
            int beep_duration_ms = 50;
            int silent_delay_ms;
            int beep_frequency_hz;

            beep_frequency_hz = (int)(3500 - (distance * 70));
            if (beep_frequency_hz < 800) beep_frequency_hz = 800;
            if (beep_frequency_hz > 3000) beep_frequency_hz = 3000;

            // DANGER ZONE : Constant High-Pitched Tone & Vibration
            if (distance < 5) {
                printf("!!! DANGER - Object at %.1f cm !!! Constant Feedback\n", distance);
                // Turn BOTH buzzer and motor ON
                set_PWM_frequency(pi, BUZZER_PIN, beep_frequency_hz);
                set_PWM_dutycycle(pi, BUZZER_PIN, 128);
                gpio_write(pi, MOTOR_PIN, 1); // Motor ON
                usleep(50000);
                continue;
            }
            
            if (distance < 10) {
                silent_delay_ms = (int)(distance * 4);
            } else if (distance < 20) {
                silent_delay_ms = (int)(distance * 6);
            } else {
                silent_delay_ms = (int)(distance * 9);
            }
            if (silent_delay_ms < 10) silent_delay_ms = 10;

            printf("Object at %.1f cm. Freq: %dHz, Rhythm: %dms ON, %dms OFF.\n",
                   distance, beep_frequency_hz, beep_duration_ms, silent_delay_ms);

            // Execute the synchronized beep and vibration
            set_PWM_frequency(pi, BUZZER_PIN, beep_frequency_hz);
            set_PWM_dutycycle(pi, BUZZER_PIN, 128); // Buzzer ON
            gpio_write(pi, MOTOR_PIN, 1);          // Motor ON
            usleep(beep_duration_ms * 1000);

            set_PWM_dutycycle(pi, BUZZER_PIN, 0); // Buzzer OFF
            gpio_write(pi, MOTOR_PIN, 0);          // Motor OFF
            usleep(silent_delay_ms * 1000);

        } else {
            // If no object is close, turn both outputs off.
            set_PWM_dutycycle(pi, BUZZER_PIN, 0);
            gpio_write(pi, MOTOR_PIN, 0);
            usleep(SLEEP_TIME); // Check file every 50ms
        }
    }
    
    return 0;
}