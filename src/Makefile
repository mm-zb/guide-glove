CC      ?= gcc
CFLAGS  ?= -std=c17 -g\
	-D_POSIX_SOURCE -D_DEFAULT_SOURCE\
	-Wall -Werror -pedantic

.SUFFIXES: .c .o

.PHONY: all clean test

all: assemble emulate

assemble: assemble.o arm_state.o
	$(CC) assemble.o arm_state.o $(LDFLAGS) $(LDLIBS) -o assemble

emulate: emulate.o arm_state.o
	$(CC) emulate.o arm_state.o $(LDFLAGS) $(LDLIBS) -o emulate

test: test_arm_state_init
	./test_arm_state_init

test_arm_state_init: test_arm_state_init.o arm_state.o
	$(CC) test_arm_state_init.o arm_state.o $(LDFLAGS) $(LDLIBS) -o test_arm_state_init

clean:
	$(RM) *.o assemble emulate arm_state test_arm_state_init