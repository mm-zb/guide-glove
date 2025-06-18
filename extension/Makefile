CC      := gcc
CFLAGS  := -Wall -Wextra -g -O2 $(shell pkg-config --cflags libpigpiod_if2)
LDFLAGS := $(shell pkg-config --libs libpigpiod_if2) -lpthread -lrt -lm
TARGETS := buzzer_feedback \
           buzzer_louder_feedback \
           buzzer_non_linear \
           buzzer_verify \
           c_distance_sensor \
           sensor_file_writer \
           buzzer_vibrator

.PHONY: all clean
all: $(TARGETS)

%: %.c
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS)

clean:
	rm -f $(TARGETS)
