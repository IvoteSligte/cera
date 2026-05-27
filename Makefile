CC = gcc

# set via CLI/justfile
BUILD ?= debug

CFLAGS_debug = -std=gnu11 -Wall -Wextra -fsanitize=address,undefined -g -O0 -rdynamic -DDEBUG_PARSER
CFLAGS_test = -std=gnu11 -Wall -Wextra -fsanitize=address,undefined -g -O0 -rdynamic -DTEST -DDEBUG_EVALUATOR
CFLAGS_lib = -std=gnu11 -Wall -Wextra
CFLAGS = $(CFLAGS_$(BUILD))

SRC_lib = $(wildcard lib/*.c)
SRC_debug = $(SRC_lib) main.c
SRC_test = $(SRC_lib) test.c

OBJ = $(SRC_$(BUILD):%.c=build/$(BUILD)/%.o)
DEP = $(OBJ:.o=.d) # header dependency files

build/$(BUILD)/%.o: %.c
	mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -MMD -MP -c $< -o $@

build/$(BUILD).out: $(OBJ)
	$(CC) $(CFLAGS) -o $@ $(OBJ)

build/lib.o: $(OBJ)
	$(CC) $(CFLAGS) -c -o $@ $(OBJ)

clean:
	rm -r build/

-include $(DEP)
