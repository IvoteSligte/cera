
run *args: build
    ./debug {{args}}

test *args: build-test
    ./test {{args}}

check:
    cc -fsyntax-only *.c lib/*.c -Wall -Wextra

format:
    make format

clean:
    make clean

