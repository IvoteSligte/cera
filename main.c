
#include "lexer.h"
#include "parser.h"

int main() {
  const char *source =
      "void lexerDoStuff(int a, int b) { printf(\"hi %s\", \"name\"); }";

  lexer_init();
  /* lexer_print_tokens(source); */
  TokenStream stream;
  if (!fill_token_stream(source, &stream)) {
    lexer_free();
    free_token_stream(stream);
    return 1;
  }
  lexer_free();
  lexer_print_token_stream(stream);

  ASTNode *ast = NULL;
  if (!parse(stream, &ast)) {
    free_token_stream(stream);
    free_ast(ast);
    panicf("Parse error.\n");
  }
  free_token_stream(stream);

  printf("Parse success.\n");
  ast_print_nodes(ast);
  
  free_ast(ast);
  return 0;
}
