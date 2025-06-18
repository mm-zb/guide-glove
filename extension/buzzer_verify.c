#include <stdio.h>
#include <pigpiod_if2.h>
#include <unistd.h>
#include <signal.h>

#include "constants.h"
// Global variable for the pigpio connection ID
int pi;

void cleanup(int signum) {
    printf("\nSignal received. Cleaning up and stopping buzzer...\n");
    // Turn off the software PWM before exiting
    if (pi >= 0) {
        set_PWM_dutycycle(pi, BUZZER_PIN, 0);
        pigpio_stop(pi);
    }
    exit(0);
}

int main(void) {
    // Set up the cleanup function to handle Ctrl+C
    signal(SIGINT, cleanup);

    // Connect to the pigpio daemon
    pi = pigpio_start(NULL, NULL);
    if (pi < 0) {
        fprintf(stderr, "Failed to connect to pigpiod daemon. Is it running?\n");
        return 1;
    }

    printf("Starting max volume test on GPIO %d.\n", BUZZER_PIN);
    printf("Generating a 440 Hz tone. Press Ctrl+C to stop.\n");

    // This is the software PWM function from pigpio
    // It is compatible with any GPIO pin

    // 1. Set the frequency for the software PWM. 440 Hz is the note 'A'
    int result = set_PWM_frequency(pi, BUZZER_PIN, 440);
    if (result < 0) {
        fprintf(stderr, "Failed to set PWM frequency.\n");
        pigpio_stop(pi);
        return 1;
    }
    
    // 2. Set the duty cycle. The range is 0-255. 128 is ~50%, which is typically loudest
    // A duty cycle of 0 turns it off
    result = set_PWM_dutycycle(pi, BUZZER_PIN, 128); 
    if (result < 0) {
        fprintf(stderr, "Failed to set PWM duty cycle.\n");
        pigpio_stop(pi);
        return 1;
    }

    // The sound will now play continuously. The program will just wait here
    // until Ctrl+C is pressed, which will trigger the cleanup() function
    while(1) {
        sleep(1);
    }
    
    return 0;
}
