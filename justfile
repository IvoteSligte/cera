
run *args:
    make debug && ./debug {{args}}

test *args:
    make test && ./test {{args}}

check:
    cc -fsyntax-only *.c lib/*.c -Wall -Wextra

format:
    make format

clean:
    make clean

