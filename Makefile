CC = gcc

# set via CLI/justfile
BUILD ?= debug

CFLAGS_debug = -Wall -Wextra -fsanitize=address,undefined -g -O0 -rdynamic -DDEBUG_PARSER
CFLAGS_test = -Wall -Wextra -fsanitize=address,undefined -g -O0 -rdynamic -DTEST
CFLAGS = $(CFLAGS_$(BUILD))

LIB_SRC = $(wildcard lib/*.c)
SRC_debug = $(LIB_SRC) main.c
SRC_test = $(LIB_SRC) test.c

OBJ = $(SRC_$(BUILD):%.c=build/$(BUILD)/%.o)

build/$(BUILD)/%.o: %.c
	mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

build/$(BUILD).out: $(OBJ)
	$(CC) $(CFLAGS) -o $@ $(OBJ)

clean:
	rm -r build/
