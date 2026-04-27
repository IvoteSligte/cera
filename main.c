
#include "analyzer.h"
#include "evaluator.h"
#include "lexer.h"
#include "parser.h"

int main() {
  char *source = read_file("test.ex");
  if (source == NULL) {
    return 1;
  }

  lexer_init();
  /* lexer_print_tokens(source); */
  TokenStream stream = {0};
  if (!fill_token_stream(source, &stream)) {
    lexer_free();
    free_token_stream(stream);
    free(source);
    return 1;
  }
  lexer_free();
  lexer_print_token_stream(stream);

  AST ast = {0};
  ParseError parse_error = {0};
  if (!parse(stream, &ast, &parse_error)) {
    print_parse_error(source, stream, parse_error);
    free_token_stream(stream);
    free_ast(&ast);
    free(source);
    return 1;
  }
  free_token_stream(stream);

  printf("Parse success.\n");
  ast_print_nodes(ast.head);

  TypeErrorArray type_errors = {0};
  if (!analyze(&ast, &type_errors)) {
    print_analyze_errors(source, type_errors);
    free_analyze_errors(&type_errors);
    free_ast(&ast);
    free(source);
    return 1;
  }
  evaluate_module(ast.head);
  free_ast(&ast);
  free(source);
  return 0;
}
