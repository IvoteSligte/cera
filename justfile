
run: build
    ./build/debug.out

test: build-test
    ./build/test.out

check:
    cc -fsyntax-only *.c test/*.c -Wall -Wextra

build:
    make build/debug.out BUILD=debug

build-test:
    make build/test.out BUILD=test

clean:
    make clean

