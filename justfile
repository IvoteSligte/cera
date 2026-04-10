
build:
    mkdir -p build/
    cc -c lexer.c -o build/lexer.o
    cc -c parser.c -o build/parser.o
    cc -c ast.c -o build/ast.o
    cc main.c build/*.o -o build/main

run: build
    ./build/main
