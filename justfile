
run *args: build
    ./debug {{args}}

test *args: build-test
    ./test {{args}}

check:
    cc -fsyntax-only *.c lib/*.c -Wall -Wextra

build:
    make debug

build-test:
    make test

build-lib:
    make lib.a

clean:
    make clean

