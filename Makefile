CC = gcc
AR = ar

CFLAGS = -std=gnu11 -Wall -Wextra -g -O0 -fPIC
LDFLAGS = -rdynamic
DEBUG_FLAGS = -fsanitize=address,undefined
LDLIBS := $(shell llvm-config --ldflags --system-libs --libs core native executionengine)

SRC = $(wildcard lib/*.c)

OBJ = $(SRC:%.c=build/%.o)
DEBUG_OBJ = $(SRC:%.c=build/debug/%.o)

DEP = $(OBJ:.o=.d) # header dependency files
DEBUG_DEP = $(DEBUG_OBJ:.o=.d) # header dependency files

build/%.o: %.c
	mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -MMD -MP -c $< -o $@

build/debug/%.o: %.c
	mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(DEBUG_FLAGS) -MMD -MP -c $< -o $@

debug: $(DEBUG_OBJ) build/debug/main.o build/debug/dep/argparse.o
	$(CC) $(LDFLAGS) $(DEBUG_FLAGS) $^ -o $@ $(LDLIBS)	

test: $(DEBUG_OBJ) build/debug/test.o
	$(CC) $(LDFLAGS) $(DEBUG_FLAGS) $^ -o $@ $(LDLIBS)	

libcera.a: $(OBJ)
	$(AR) rcs libcera.a $(OBJ)

lib: libcera.a

cera-ls: libcera.a
	cd lsp && cargo build && cp target/debug/cera-ls ..

lsp: cera-ls

format:
	clang-format -i $(SRC) main.c test.c

clean:
	rm -rf build/
	rm -f debug test
	cd lsp && rm -f cera-lsp && cargo clean

.PHONY: clean format lib lsp cera-ls

-include $(DEP)

