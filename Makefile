CC = gcc
AR = ar

CFLAGS = -std=gnu11 -Wall -Wextra -g -O0 -fsanitize=address,undefined
LDFLAGS = -rdynamic -fsanitize=address,undefined
LDLIBS := $(shell llvm-config --ldflags --system-libs --libs core native executionengine)

SRC = $(wildcard lib/*.c)
OBJ = $(SRC:%.c=build/%.o)
DEP = $(OBJ:.o=.d) # header dependency files

build/%.o: %.c
	mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -MMD -MP -c $< -o $@

debug: $(OBJ) main.o
	$(CC) $(LDFLAGS) -o $@ $(OBJ) main.o $(LDLIBS)

test: $(OBJ) test.o
	$(CC) $(LDFLAGS) -o $@ $(OBJ) test.o $(LDLIBS)

libceam.a: $(OBJ)
	$(AR) rcs libceam.a $(OBJ)

lib: libceam.a

ceam-lsp: libceam.a
	cd lsp && cargo build && cp target/debug/ceam-lsp ..

lsp: ceam-lsp

format:
	clang-format -i $(SRC) main.c test.c

clean:
	rm -rf build/
	rm -f debug test
	cd lsp && rm -f ceam-lsp && cargo clean

.PHONY: clean format lib lsp

-include $(DEP)

