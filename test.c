
#include <dirent.h> // TODO: cross-platform
#include <regex.h>  // TODO: cross-platform
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include "lib/analyzer.h"
#include "lib/evaluator.h"
#include "lib/parser.h"
#include "lib/util.h"

// Buffer for capturing program stdout.
char *stdout_buffer = NULL;
size_t stdout_size = 0;

// Used by lib/evaluator.c to capture stdout.
void print_string(const char *text, size_t length) {
  stdout_buffer = realloc(stdout_buffer, stdout_size + length);
  memcpy(&stdout_buffer[stdout_size], text, length);
  stdout_size += length;
}

void compile_regex(const char *pattern, regex_t *regex) {
  int result = regcomp(regex, pattern, REG_EXTENDED);
  if (result) {
    char errbuf[100];
    regerror(result, regex, errbuf, 100);
    panicf("Failed to compile regex `%s`. Error: %s", pattern, errbuf);
  }
}

bool match_regex(regex_t *regex, const char *string, regmatch_t *out_match) {
  int result = regexec(regex, string, 1, out_match, 0);
  if (!IS_ONE_OF(result, REG_NOERROR, REG_NOMATCH)) {
    char errbuf[100];
    regerror(result, regex, errbuf, 100);
    panicf("Failed to run regex. Error: %s", errbuf);
  }
  return result == REG_NOERROR; // match
}

#define TRY($expr)                                                             \
  if (!($expr)) {                                                              \
    *failed_line = __LINE__;                                                   \
    *failed_expr = #$expr;                                                     \
    result = false;                                                            \
    goto end;                                                                  \
  }

bool test(const char *source, const char *expected_output) {
  TokenStream stream = {0};
  AST ast = {0};
  LexError lex_error = {0};
  ParseError parse_error = {0};
  AnalyzeErrorArray type_errors = {0};

  if (!(fill_token_stream(source, &stream, &lex_error))) {
    print_lex_error(lex_error);
    free_token_stream(&stream);
    return false;
  }
  if (!parse_token_stream(stream, &ast, &parse_error)) {
    print_parse_error(source, stream, parse_error);
    free_token_stream(&stream);
    free_ast(&ast);
    return false;
  }
  if (!analyze(&ast, &type_errors)) {
    print_analyze_errors(source, type_errors);
    free_analyze_errors(&type_errors);
    free_token_stream(&stream);
    free_ast(&ast);
    return false;
  }
  evaluate_module(ast.head); // TODO: compare output with expected_output

  if (stdout_size != strlen(expected_output) ||
      (stdout_buffer != NULL &&
       strncmp(stdout_buffer, expected_output, stdout_size) != 0)) {
    eprintf("Expected output does not match actual output.\n");
    eprintf("Expected (%zu bytes): `%s`\n", strlen(expected_output),
            expected_output);
    eprintf("Actual   (%zu bytes): `%.*s`\n", stdout_size, (int)stdout_size,
            stdout_buffer);
    free(stdout_buffer);
    free_token_stream(&stream);
    free_ast(&ast);
    return false;
  }
  free(stdout_buffer);
  stdout_buffer = NULL;
  stdout_size = 0;

  free_token_stream(&stream);
  free_ast(&ast);
  return true;
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
    regmatch_t match;
    if (!has_regex || match_regex(&regex, name, &match)) {
      char *source = read_file(path);
      if (source == NULL) {
        continue;
      }
      char *separator = strstr(source, "===");
      const char *expected_output = "";
      if (separator != NULL) {
        *separator = '\0';               // make sure source is zero-delimited
        expected_output = separator + 4; // after ===\n
      }
      num_tests++;
      printf("- running test %s\n", name);
      if (test(source, expected_output)) {
        num_succeeded++;
        printf("- test %s succeeded\n", name);
      } else {
        printf("- test %s failed\n", name);
      }
      free(source);
    }
    free(name);
  }
  closedir(dir);

  printf("[%zu/%zu] tests succeeded\n", num_succeeded, num_tests);

  if (has_regex) {
    regfree(&regex);
  }
  return num_succeeded < num_tests ? 1 : 0;
}
