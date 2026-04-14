
build:
    mkdir -p build/
    cc -c alloc.c -o build/alloc.o
    cc -c lexer.c -o build/lexer.o
    cc -c parser.c -o build/parser.o
    cc -c ast.c -o build/ast.o
    cc -c analyzer.c -o build/analyzer.o
    cc main.c build/*.o -o build/main -Wall -Wextra -fsanitize=address,undefined -g

run: build
    ./build/main

