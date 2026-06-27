
#include <ctype.h>
#include <dirent.h> // TODO: cross-platform
#include <llvm-c/Core.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include "lib/api.h"
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

#define TRY($expr)                                                             \
  if (!($expr)) {                                                              \
    *failed_line = __LINE__;                                                   \
    *failed_expr = #$expr;                                                     \
    result = false;                                                            \
    goto end;                                                                  \
  }

typedef struct {
  char *source;
  char *expected_output;
  int expected_status;
} TestFile;

bool test(const char *source) {
  AST ast = {0};

  CompileErrors errors = compile_and_run(source);
  if (errors.length > 0) {
    print_compile_errors(errors);
    free_ast(&ast);
    free_compile_errors(&errors);
    return false;
  }
  free_ast(&ast);
  return true;
}

char *map_escape_chars(const char *src, size_t length) {
  char *dst = calloc(length + 1, 1);
  for (size_t src_i = 0, dst_i = 0; src_i < length; src_i++, dst_i++) {
    if (src_i + 1 < length && src[src_i] == '\\' && src[src_i + 1] == 'n') {
      dst[dst_i] = '\n';
      src_i++;
    } else {
      dst[dst_i] = src[src_i];
    }
  }
  return dst;
}

// Test hints:
//  OUTPUT: "output text"
//  STATUS: <integer>
TestFile read_test_file(const char *path) {
  TestFile file = {0};
  file.source = read_file(path);
  if (file.source == NULL) {
    return file;
  }
  char *output_prefix = strstr(file.source, "OUTPUT");
  if (output_prefix != NULL) {
    char *output_start_quote = strchr(output_prefix, '"');
    assert(output_start_quote != NULL);
    char *output_start = output_start_quote + 1;
    char *output_end = strchr(output_start, '"');
    assert(output_end != NULL);
    size_t length = output_end - output_start;
    file.expected_output = map_escape_chars(output_start, length);
  } else {
    file.expected_output = strdup("");
  }
  file.expected_status = 0;
  char *status_str = strstr(file.source, "STATUS");
  if (status_str != NULL) {
    for (; !isdigit(*status_str); status_str++)
      ;
    for (; isdigit(*status_str); status_str++) {
      file.expected_status *= 10;
      file.expected_status += *status_str - '0';
    }
  }
  return file;
}

void free_test_file(TestFile *test_file) {
  free(test_file->source);
  free(test_file->expected_output);
  *test_file = (TestFile){0};
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

typedef struct {
  const char *file;
  bool show_logs;
  const char *pattern;
} Options;

#define IS_OPTION($arg, $short, $long)                                         \
  (!finished_options && (str_eq($arg, $long) || str_eq($arg, $short)))

Options parse_options(int argc, const char *argv[]) {
  Options options = {0};
  bool finished_options = false;

  for (int i = 1; i < argc; i++) {
    if (str_eq(argv[i], "--")) {
      finished_options = true;
      continue;
    }
    if (IS_OPTION(argv[i], "-h", "--help")) {
      eprintf("Usage: %s [pattern] [option]...\n", argv[0]);
      eprintf("\t-h, --help\n");
      eprintf("\t\tPrint this menu.\n");
      eprintf("\n");
      eprintf("\t-f, --file <file_path>\n");
      eprintf("\t\tExecute a test by file path. Status indicates success.\n");
      eprintf("\n");
      eprintf("\t-s, --show-logs\n");
      eprintf("\t\tPrints compiler logs to the console instead of to "
              "'/tmp/cm-test-<test_name>'.\n");
      eprintf("\n");
      exit(0);
    }
    if (IS_OPTION(argv[i], "-f", "--file")) {
      if (argc < i + 2) {
        eprintf("No file specified for '--file' option.\n");
        exit(1);
      }
      if (argv[i + 1][0] == '-') {
        eprintf("File expected following '--file', but option found.\n");
        exit(1);
      }
      if (options.file != NULL) {
        eprintf("Duplicate file argument.\n");
        exit(1);
      }
      options.file = argv[i + 1];
      i++;
      continue;
    }
    if (IS_OPTION(argv[i], "-s", "--show-logs")) {
      options.show_logs = true;
      continue;
    }
    if (!finished_options && argv[i][0] == '-') {
      eprintf("Invalid option '%s'\n", argv[i]);
      exit(1);
    }
    if (options.pattern != NULL) {
      eprintf("Duplicate pattern parameter.\n");
      exit(1);
    }
    options.pattern = argv[i];
  }
  if (options.file != NULL && options.pattern != NULL) {
    eprintf("Cannot mix file and pattern options.\n");
    exit(1);
  }
  return options;
}

const char *status_message(int status) {
  switch (status) {
  case 124:
    return "timeout";
  case 132:
    return "illegal instruction";
  case 134:
    return "panic";
  case 139:
    return "segfault";
  case 0:
    return "ok";
  default:
    return "crash";
  }
}

// The first argument is an optional regex pattern that matches test names to
// run.
int main(int argc, const char *argv[]) {
  Options options = parse_options(argc, argv);

  if (options.file != 0) {
    TestFile test_file = read_test_file(options.file);
    int status = test(test_file.source) ? 0 : 1;
    free_test_file(&test_file);
    return status;
  }
  size_t num_tests = 0;
  size_t num_succeeded = 0;
  DIR *dir = opendir("tests/");
  if (dir == NULL) {
    printf("Could not open tests/ directory.");
    return 1;
  }
  struct dirent *dir_entry = NULL;
  // 256 is the max size of a filename on Linux
  char path[sizeof("tests/") - 1 + 256] = "tests/";
  while ((dir_entry = readdir(dir)) != NULL) {
    const char *file_name = dir_entry->d_name;
    strcpy(&path[sizeof("tests/") - 1], file_name);
    const char *file_extension = strrchr(file_name, '.');
    if (!str_eq(file_extension, FILE_EXTENSION)) {
      continue;
    }
    char *name = file_extension == NULL
                     ? strdup(file_name)
                     : strndup(file_name, file_extension - file_name);
    if (options.pattern == NULL || strstr(name, options.pattern)) {
      TestFile test_file = read_test_file(path);
      assert(test_file.source != NULL);
      num_tests++;

      // Run the test as a separate process.
      // Only capture stdout, leaving stderr as log file in /tmp/cm-test-<name>
      // FIXME: when options.show_logs is true the status code is always 0?
      char *cmd = options.show_logs
                      ? ssprintf("timeout 1 %s --file %s", argv[0], path)
                      : ssprintf("timeout 1 %s --file %s 2>/tmp/cera-test-%s",
                                 argv[0], path, name);
      FILE *stdout = popen(cmd, "r"); // TODO: cross-platform
      free(cmd);
      char *stdout_string = read_stream(stdout);
      int status = WEXITSTATUS(pclose(stdout));
      if (status != test_file.expected_status) {
        printf("- test %-21s " RED("failed") " [%3d] (%s) \n", name, status,
               status_message(status));
        goto cont;
      }
      // Compare output with expected output.
      const char *expected_output = test_file.expected_output;
      size_t stdout_size = strlen(stdout_string);
      if (!str_eq(stdout_string, expected_output)) {
        eprintf("- test %-21s " RED("failed") " [%3d] (output mismatch) \n",
                name, status);
        eprintf("Expected output does not match actual output.\n");
        eprintf("Expected (%zu bytes): `%s`\n", strlen(expected_output),
                expected_output);
        eprintf("Actual   (%zu bytes): `%.*s`\n", stdout_size, (int)stdout_size,
                stdout_string);
        goto cont;
      }
      num_succeeded++;
      printf("- test %-21s " GREEN("succeeded") "\n", name);
    cont:
      free_test_file(&test_file);
      free(stdout_string);
    }
    free(name);
  }
  closedir(dir);

  printf("[%zu/%zu] tests succeeded\n", num_succeeded, num_tests);

  return num_succeeded < num_tests ? 1 : 0;
}
