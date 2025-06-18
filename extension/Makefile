CC = gcc
CFLAGS = -Wall -Wextra -g -O2

LDFLAGS = -lpigpiod_if2 -lpthread -lrt -lm

TARGETS = \
    buzzer_feedback \
    buzzer_louder_feedback \
    buzzer_non_linear \
    buzzer_verify \
    c_distance_sensor \
    sensor_file_writer

.PHONY: all
all: $(TARGETS)

buzzer_feedback: buzzer_feedback.c constants.h
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS)

buzzer_louder_feedback: buzzer_louder_feedback.c buzzer.h constants.h
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS)

buzzer_non_linear: buzzer_non_linear.c constants.h
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS)

buzzer_verify: buzzer_verify.c buzzer.h constants.h
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS)

c_distance_sensor: c_distance_sensor.c constants.h
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS)

sensor_file_writer: sensor_file_writer.c constants.h
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS)

.PHONY: clean
clean:
	rm -f $(TARGETS)