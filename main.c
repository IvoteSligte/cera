
#include "lexer.h"
#include "parser.h"

char *read_file(const char *path) {
  FILE *fptr = fopen(path, "r");

  if (fptr == NULL) {
    perror("Failed to open file:");
    return NULL;
  }
  if (fseek(fptr, 0L, SEEK_END) != 0) {
    perror("Failed to seek to end of file:");
    return NULL;
  }
  size_t size = ftell(fptr);
  if (fseek(fptr, 0L, SEEK_SET) != 0) {
    perror("Failed to seek to start of file:");
    return NULL;
  }
  char *data = malloc(size + 1);

  if (fread(data, 1, size, fptr) != size) {
    perror("Reading file returned an unexpected number of bytes:");
    free(data);
    return NULL;
  }
  data[size] = '\0';

  if (fclose(fptr) != 0) {
    perror("Failed to close file:");
  }
  return data;
}

int main() {
  char *source = read_file("test.ex");
  if (source == NULL) {
    return 1;    
  }

  lexer_init();
  /* lexer_print_tokens(source); */
  TokenStream stream;
  if (!fill_token_stream(source, &stream)) {
    lexer_free();
    free_token_stream(stream);
    free(source);
    return 1;
  }
  lexer_free();
  lexer_print_token_stream(stream);

  AST ast = {0};
  ParseError error_data;
  if (!parse(stream, &ast, &error_data)) {
    print_parse_error(source, stream, error_data);
    free_token_stream(stream);
    free_ast(&ast);
    free(source);
    panicf("Parse error.\n");
  }
  free_token_stream(stream);

  printf("Parse success.\n");
  ast_print_nodes(ast.head);

  free_ast(&ast);
  free(source);
  return 0;
}
