CC = gcc

# CFLAGS = -std=gnu11 -Wall -Wextra -fsanitize=address,undefined -g -O0 -rdynamic
CFLAGS = -std=gnu11 -Wall -Wextra -g -O0 -rdynamic

SRC = $(wildcard lib/*.c)
OBJ = $(SRC:%.c=build/%.o)
DEP = $(OBJ:.o=.d) # header dependency files

build/%.o: %.c
	mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -MMD -MP -c $< -o $@

debug: $(OBJ) main.o
	$(CC) $(CFLAGS) -o $@ $(OBJ) main.o

test: $(OBJ) test.o
	$(CC) $(CFLAGS) -o $@ $(OBJ) test.o

clean:
	rm -rf build/
	rm -f debug test lsp lib.a

.PHONY: clean

-include $(DEP)

