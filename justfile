
check:
    cc -fsyntax-only *.c -Wall -Wextra

build:
    make

run: build
    ./build/main

clean:
    make clean

