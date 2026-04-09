
build-parser:
    bison -d parser.y
    flex lexer.l


