
build:
    cc -c lexer.c
    cc -c parser.c
    cc -c ast.c
    cc main.c lexer.o parser.o ast.o -o main

run: build
    ./main
