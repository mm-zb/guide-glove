CC      ?= gcc
CFLAGS  ?= -std=c17 -g\
	-D_POSIX_SOURCE -D_DEFAULT_SOURCE\
	-Wall -Werror -pedantic

.SUFFIXES: .c .o

.PHONY: all clean test

all: assemble emulate

ASS_SRCS = assemble.c tokenizer.c symbol_table.c assemble_dp.c
ASS_OBJS = $(ASS_SRCS:.c=.o)

assemble: $(ASS_OBJS)
	$(CC) $(ASS_OBJS) $(LDFLAGS) $(LDLIBS) -o assemble

emulate: emulate.o arm_state.o decoder.o executor.o mem_branch_executor.o addressing.o dp_executor.o shifts.o
	$(CC) emulate.o arm_state.o decoder.o executor.o mem_branch_executor.o addressing.o dp_executor.o shifts.o $(LDFLAGS) $(LDLIBS) -o emulate

test: test_arm_state_init
	./test_arm_state_init

test_arm_state_init: test_arm_state_init.o arm_state.o
	$(CC) test_arm_state_init.o arm_state.o $(LDFLAGS) $(LDLIBS) -o test_arm_state_init

clean:
	$(RM) *.o assemble emulate test_arm_state_init

tokenizer.o: tokenizer.c tokenizer.h
	$(CC) $(CFLAGS) -c tokenizer.c

symbol_table.o: symbol_table.c symbol_table.h
	$(CC) $(CFLAGS) -c symbol_table.c

assemble.o: assemble.c tokenizer.h symbol_table.h
	$(CC) $(CFLAGS) -c assemble.c