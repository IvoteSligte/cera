CC = gcc

# CFLAGS = -std=gnu11 -Wall -Wextra -fsanitize=address,undefined -g -O0 -rdynamic
CFLAGS = -std=gnu11 -Wall -Wextra -g -O0 -rdynamic

SRC = $(wildcard lib/*.c)
OBJ = $(SRC:%.c=build/%.o)
DEP = $(OBJ:.o=.d) # header dependency files

build/%.o: %.c
	mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -MMD -MP -c $< -o $@

debug: $(OBJ)
	$(CC) $(CFLAGS) main.c -o $@ $(OBJ)

test: $(OBJ)
	$(CC) $(CFLAGS) test.c -o $@ $(OBJ)

lib.a: $(OBJ)
	ar rcs lib.a $(OBJ)

clean:
	rm -r build/
	rm debug test lib.a

.PHONY: clean

-include $(DEP)

