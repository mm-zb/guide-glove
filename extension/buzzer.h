#ifndef BUZZER_H
#define BUZZER_H

#include <stdio.h>
#include <stdlib.h> 
#include <pigpiod_if2.h>
#include <unistd.h> 

#include "constants.h"

void cleanup(int signum) {
    printf("\nSignal received. Cleaning up and stopping buzzer...\n");
    if (pi >= 0) {
        set_PWM_dutycycle(pi, BUZZER_PIN, 0); // Ensure buzzer is off
        pigpio_stop(pi);
    }
    exit(0);
}

#endif