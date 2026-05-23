
#include <dirent.h> // TODO: cross-platform
#include <regex.h>  // TODO: cross-platform
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include "lib/analyzer.h"
#include "lib/ast.h"
#include "lib/evaluator.h"
#include "lib/parser.h"
#include "lib/util.h"

// TODO: no colors on terminals that do not support it?

// ANSI color codes
#define COL(num, s) "\e[0;" #num "m" s "\e[0m"
#define BLACK(s) COL(30, s)
#define RED(s) COL(31, s)
#define GREEN(s) COL(32, s)
#define YELLOW(s) COL(33, s)
#define BLUE(s) COL(34, s)
#define MAGENTA(s) COL(35, s)
#define CYAN(s) COL(36, s)
#define WHITE(s) COL(37, s)

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

typedef struct {
  char *source;
  const char *expected_output;
} TestFile;

bool evaluate(const char *source) {
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
  ast_print_nodes(ast.head);
  if (!analyze(&ast, &type_errors)) {
    print_analyze_errors(source, type_errors);
    free_analyze_errors(&type_errors);
    free_token_stream(&stream);
    free_ast(&ast);
    return false;
  }
  evaluate_module(ast.head);

  free_token_stream(&stream);
  free_ast(&ast);
  return true;
}

TestFile read_test_file(const char *path) {
  TestFile file = {0};
  file.source = read_file(path);
  if (file.source == NULL) {
    return file;
  }
  char *separator = strstr(file.source, "===");
  if (separator != NULL) {
    *separator = '\0';                    // make sure source is zero-delimited
    file.expected_output = separator + 4; // after ===\n
  } else {
    file.expected_output = strchr(file.source, '\0');
  }
  return file;
}

void free_test_file(TestFile *test_file) {
  free(test_file->source);
  *test_file = (TestFile){0};
}

char *dyn_strcat(char *dest, const char *source) {
  dest = realloc(dest, strlen(dest) + strlen(source) + 1);
  return strcat(dest, source);
}

char *read_stream(FILE *stream) {
  assert(stream != NULL);
  char *string = malloc(256);
  string[0] = '\0';
  char buffer[256];

  while (!feof(stream)) {
    if (fgets(buffer, 256, stream) != NULL) {
      string = realloc(string, strlen(string) + strlen(buffer) + 1);
      strcat(string, buffer);
    }
  }
  return string;
}

// The first argument is an optional regex pattern that matches test names to
// run.
int main(int argc, const char *argv[]) {
  size_t num_tests = 0;
  size_t num_succeeded = 0;
  regex_t regex = {0};
  bool has_regex = false;
  if (argc > 1) {
    if (strcmp(argv[1], "--file") == 0) {
      if (argc < 3) {
        eprintf("Usage: %s --file <test_file>\n", argv[0]);
        return 1;
      }
      const char *path = argv[2];
      TestFile test_file = read_test_file(path);
      int status = evaluate(test_file.source) ? 0 : 1;
      free_test_file(&test_file);
      return status;
    }
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
  // 256 is the max size of a filename on Linux
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
      TestFile test_file = read_test_file(path);
      assert(test_file.source != NULL);
      num_tests++;

      // Only captures stdout, leaving stderr as log file in /tmp/cm-test-<name>
      char *cmd = strdup(argv[0]);
      cmd = dyn_strcat(cmd, " --file ");
      cmd = dyn_strcat(cmd, path);
      cmd = dyn_strcat(cmd, " 2>/tmp/cm-test-");
      cmd = dyn_strcat(cmd, name);

      FILE *stdout = popen(cmd, "r"); // TODO: cross-platform
      free(cmd);
      char *stdout_string = read_stream(stdout);
      int status = pclose(stdout);
      if (status != 0) {
        printf("- test %-20s " RED("failed") "\n", name);
        free_test_file(&test_file);
        free(stdout_string);
        free(name);
        continue;
      }
      // compare output with expected output
      const char *expected_output = test_file.expected_output;
      size_t stdout_size = strlen(stdout_string);
      if (strcmp(stdout_string, expected_output) != 0) {
        eprintf("Expected output does not match actual output.\n");
        eprintf("Expected (%zu bytes): `%s`\n", strlen(expected_output),
                expected_output);
        eprintf("Actual   (%zu bytes): `%.*s`\n", stdout_size, (int)stdout_size,
                stdout_string);
        printf("- test %-20s " RED("failed") "\n", name);
        free_test_file(&test_file);
        free(stdout_string);
        free(name);
        continue;
      }
      num_succeeded++;
      printf("- test %-20s " GREEN("succeeded") "\n", name);
      free_test_file(&test_file);
      free(stdout_string);
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
