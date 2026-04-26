CC = gcc
CFLAGS = -Wall -Wextra -fsanitize=address,undefined -g -O0 -rdynamic -DDEBUG_PARSER

SRC = $(wildcard *.c)
OBJ = $(SRC:%.c=build/%.o)

build/%.o: %.c
	mkdir -p build/
	$(CC) $(CFLAGS) -c $< -o $@

build/main: $(OBJ)
	$(CC) $(CFLAGS) -o $@ $(OBJ)

clean:
	rm build/*
