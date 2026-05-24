
#include "lib/analyzer.h"
#include "lib/evaluator.h"
#include "lib/lexer.h"
#include "lib/parser.h"

int main() {
  char *source = read_file("test.ex");
  if (source == NULL) {
    return 1;
  }

#ifdef DEBUG_EVALUATOR
  program_source = source;
#endif

  LexError lex_error = {0};
  TokenStream stream = {0};
  if (!fill_token_stream(source, &stream, &lex_error)) {
    print_lex_error(lex_error);
    free_token_stream(&stream);
    free(source);
    return 1;
  }
  print_token_stream(stream);

  AST ast = {0};
  ParseError parse_error = {0};
  if (!parse_token_stream(stream, &ast, &parse_error)) {
    print_parse_error(source, stream, parse_error);
    free_token_stream(&stream);
    free_ast(&ast);
    free(source);
    return 1;
  }
  free_token_stream(&stream);

  eprintf("Parse success.\n");
  ast_print_nodes(ast.head);

  AnalyzeErrorArray type_errors = {0};
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
