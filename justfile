
build:
    cc -c lexer.c -o lexer.o
    cc main.c lexer.o -o main

run: build
    ./main
