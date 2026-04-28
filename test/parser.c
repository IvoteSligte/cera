
#include <stdbool.h>
#include <stdio.h>

#include "../parser.h"
#include "../regex.h"

#define TEST($name, $source)                                                   \
  {                                                                            \
    regmatch_t match;                                                          \
    if (!has_regex || match_regex(&regex, #$name, &match)) {                   \
      num_tests++;                                                             \
      printf("- running test " #$name " \n");                                  \
      if (test($source)) {                                                     \
        num_succeeded++;                                                       \
        printf("- test " #$name " succeeded\n");                               \
      } else {                                                                 \
        printf("- test " #$name " failed\n");                                  \
      }                                                                        \
    }                                                                          \
  }

typedef enum {
  LEXING,
  PARSING,
  DONE,
} Stage;

#define TRY($expr)                                                             \
  if (!($expr)) {                                                              \
    *failed_line = __LINE__;                                                   \
    *failed_expr = #$expr;                                                     \
    result = false;                                                            \
    goto end;                                                                  \
  }

bool test(const char *source) {
  AST ast = {0};
  LexError lex_error = {0};
  ParseError parse_error = {0};
  bool result = false;
  TokenStream stream = {0};
  if (!(fill_token_stream(source, &stream, &lex_error))) {
    print_lex_error(lex_error);
    goto end;
  }
  if (!parse_token_stream(stream, &ast, &parse_error)) {
    print_parse_error(source, stream, parse_error);
    goto end;
  }
  result = DONE;
end:
  free_token_stream(&stream);
  free_ast(&ast);
  return result;
}

// The first argument is an optional regex pattern that matches test names to run.
int main(int argc, const char *argv[]) {
  size_t num_tests = 0;
  size_t num_succeeded = 0;
  regex_t regex = {0};
  bool has_regex = false;
  if (argc > 1) {
    const char *pattern = argv[1];
    compile_regex(pattern, &regex);
    has_regex = true;
  }
  lexer_init();

  TEST(empty_main, "main :: () {}");
  TEST(literals, "main :: () { 0; 100; 592391; \"a string literal\"; }");
  TEST(binary_exprs, "main :: () { 5 + 6; 9 - 1 * 4 / 2; }");
  printf("[%zu/%zu] tests succeeded\n", num_succeeded, num_tests);

  lexer_free();
  if (has_regex) {
    free_regex(&regex);
  }
  return num_succeeded < num_tests ? 1 : 0;
}
