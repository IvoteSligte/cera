
#include <dirent.h> // TODO: cross-platform
#include <stdbool.h>
#include <stdio.h>

#include "lib/parser.h"
#include "lib/regexp.h"
#include "lib/util.h"

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

// The first argument is an optional regex pattern that matches test names to
// run.
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

  DIR *dir = opendir("test/");
  if (dir == NULL) {
    printf("Could not open test/ directory.");
    return 1;
  }
  lexer_init();

  struct dirent *dir_entry = NULL;
  char path[5 + 256] = "test/";
  while ((dir_entry = readdir(dir)) != NULL) {
    const char *file_name = dir_entry->d_name;
    strcpy(&path[5], file_name);
    const char *file_extension = strrchr(file_name, '.');
    if (strcmp(file_extension, FILE_EXTENSION) != 0) {
      continue;
    }
    char *name = file_extension == NULL
                     ? strdup(file_name)
                     : strndup(file_name, file_extension - file_name);
    char *source = read_file(path);
    if (source == NULL) {
      continue;
    }
    regmatch_t match;
    if (!has_regex || match_regex(&regex, name, &match)) {
      num_tests++;
      printf("- running test %s\n", name);
      if (test(source)) {
        num_succeeded++;
        printf("- test %s succeeded\n", name);
      } else {
        printf("- test %s failed\n", name);
      }
    }
    free(source);
    free(name);
  }
  closedir(dir);

  printf("[%zu/%zu] tests succeeded\n", num_succeeded, num_tests);

  lexer_free();
  if (has_regex) {
    free_regex(&regex);
  }
  return num_succeeded < num_tests ? 1 : 0;
}
