CC = gcc
CFLAGS = -Wall -Wextra -fsanitize=address,undefined -g

SRC = $(wildcard *.c)
OBJ = $(SRC:src/%.c=build/%.o)

build/%.o: %.c
	mkdir -p build/
	$(CC) $(CFLAGS) -c $< -o $@

build/main: $(OBJ)
	$(CC) $(CFLAGS) -o $@ $(OBJ)

clean:
	rm build/*
