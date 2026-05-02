
run *args: build
    ./build/debug.out {{args}}

test *args: build-test
    ./build/test.out {{args}}

check:
    cc -fsyntax-only *.c lib/*.c -Wall -Wextra

build:
    make build/debug.out BUILD=debug

build-test:
    make build/test.out BUILD=test

clean:
    make clean

