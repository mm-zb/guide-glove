#include <stdio.h>
#include <pigpiod_if2.h>
#include <unistd.h>    // for usleep()

#include "constants.h"


int main(void) {
    // connect to local pigpiod daemon
    int pi = pigpio_start(NULL, NULL);
    if (pi < 0) {
        fprintf(stderr, "Failed to connect to pigpiod daemon\n");
        return 1;
    }

    // configure pins
    set_mode(pi, TRIG_PIN, PI_OUTPUT);
    set_mode(pi, ECHO_PIN, PI_INPUT);
    gpio_write(pi, TRIG_PIN, 0);
    sleep(1);  // let sensor settle

    printf("Distance measurement in progress. Ctrl+C to stop.\n");

    while (1) {
        // 1. trigger
        gpio_write(pi, TRIG_PIN, 1);
        usleep(10);               // 10 µs
        gpio_write(pi, TRIG_PIN, 0);

        // 2. wait for echo to go high
        unsigned int startTick, endTick;
        unsigned int timeout = get_current_tick(pi) + 100000; // 0.1 s
        while (gpio_read(pi, ECHO_PIN) == 0) {
            if (get_current_tick(pi) > timeout) { startTick = 0; break; }
        }
        startTick = get_current_tick(pi);

        // 3. wait for echo to go low
        timeout = startTick + 100000;
        while (gpio_read(pi, ECHO_PIN) == 1) {
            if (get_current_tick(pi) > timeout) { endTick = 0; break; }
        }
        endTick = get_current_tick(pi);

        // 4. compute distance
        if (endTick > startTick) {
            float pulse_us = (float)(endTick - startTick);
            float dist_cm  = (pulse_us / 1e6f) * 17150.0f;
            printf("distance: %.2f cm\n", dist_cm);
        } else {
            printf("Measurement failed or out of range.\n");
        }

        usleep(SLEEP_TIME);
    }

    pigpio_stop(pi);
    return 0;
}