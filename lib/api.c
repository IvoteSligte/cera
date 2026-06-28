#include "api.h"
#include "analyzer.h"
#include "generator.h"
#include "lexer.h"
#include "llvm.h"
#include "parser.h"
#include "util.h"

#ifdef __linux__
#include <sys/stat.h>
/* #elif _WIN32_ */
/* TODO */
#else
#error "TODO: cross-platform"
#endif

typedef CompileError Error;
typedef CompileErrors Errors;

Error new_error(char *message, size_t line, size_t column, size_t length) {
  return (Error){
      .message = message, .line = line, .column = column, .length = length};
}

void push_error(Errors *errors, Error error) {
  errors->data = realloc(errors->data, sizeof(Error) * (errors->length + 1));
  errors->data[errors->length] = error;
  errors->length += 1;
}

bool parse_and_analyze(const char *source, AST *out_ast, Errors *out_errors) {
  *out_ast = (AST){0};
  TokenStream stream = {0};
  LexError lex_error = {0};
  ParseError parse_error = {0};
  AnalyzeErrorArray analyze_errors = {0};
  Error error = {0};

  if (!(fill_token_stream(source, &stream, &lex_error))) {
    get_lex_error_info(lex_error, &error);
    push_error(out_errors, error);

    free_token_stream(&stream);
    return false;
  }
  if (!parse_token_stream(stream, out_ast, &parse_error)) {
    get_parse_error_info(source, stream, parse_error, &error);
    push_error(out_errors, error);

    free_token_stream(&stream);
    free_ast(out_ast);
    return false;
  }
  if (!analyze(out_ast, &analyze_errors)) {
    for (size_t i = 0; i < analyze_errors.length; i++) {
      get_analyze_error_info(source, analyze_errors.data[i], &error);
      push_error(out_errors, error);
    }
    free_analyze_errors(&analyze_errors);
    free_token_stream(&stream);
    free_ast(out_ast);
    return false;
  }
  free_token_stream(&stream);
  return true;
}

Errors compile_to_object_file(const char *source, const char *output_file) {
  Errors errors = {0};
  AST ast = {0};
  if (parse_and_analyze(source, &ast, &errors)) {
    LLVMContextRef ctx = LLVMContextCreate();
    LLVMModuleRef mod = generate_llvm(ctx, &ast);
    llvm_compile_to_file(mod, output_file);
    LLVMDisposeModule(mod);
    LLVMContextDispose(ctx);
  }
  free_ast(&ast);
  return errors;
}

#define REMOVE_TEMP_FILES                                                      \
  remove(object_file);                                                         \
  remove(exec_file)

Errors compile_and_run(const char *source) {
  char object_file[] = "/tmp/cera-XXXXXX.o";
  char exec_file[] = "/tmp/cera-XXXXXX";
  if (!create_temp_file(object_file) || !create_temp_file(exec_file)) {
    REMOVE_TEMP_FILES;
    panicf("TODO: handle temp file creation failure");
  }
  Errors errors = compile_to_object_file(source, object_file);
  if (errors.length > 0) {
    REMOVE_TEMP_FILES;
    return errors;
  }
  if (!link_to_executable((const char **)&object_file, 1, exec_file)) {
    REMOVE_TEMP_FILES;
    return (Errors){0};
  }
  // Give user read|write|execute permissions.
  if (chmod(exec_file, S_IRUSR | S_IWUSR | S_IXUSR) != 0) {
    REMOVE_TEMP_FILES;
    pprintf("Failed to make file executable.");
    // NOTE: should this just crash because of how unlikely it is?
    panicf("TODO: handle file chmod failure");
  }
  const char *argv[] = {exec_file, NULL};
  // NOTE: should command status be handled? maybe returned so the running
  // program can give the same status?
  run_command(argv);
  REMOVE_TEMP_FILES;
  return (Errors){0};
}

Errors diagnose(const char *source) {
  Errors errors = {0};
  AST ast = {0};
  parse_and_analyze(source, &ast, &errors);
  free_ast(&ast);
  return errors;
}

static bool is_empty_string(const char *s) { return s == NULL || s[0] == '\0'; }

// TODO: make these paths relative to the Cera compiler installation directory
#define STARTUP_PATH "startup.c"
#define BUILTIN_PATH "lib/builtin.c"

bool link_to_executable(const char **object_files, size_t num_object_files,
                        const char *output_file) {
  eprintf("Linking to binary.\n");
  assert(!is_empty_string(output_file));

  size_t argc = 5 + num_object_files;
  const char **argv = calloc(argc + 1, sizeof(char *)); // +1 for trailing NULL
  argv[0] = "cc";
  argv[1] = STARTUP_PATH;
  argv[2] = BUILTIN_PATH;
  argv[3] = "-o";
  argv[4] = output_file;

  for (size_t i = 0; i < num_object_files; i++) {
    const char *object_file = object_files[i];
    assert(!is_empty_string(object_file));
    argv[5 + i] = object_file;
  }
  eprintf("Linker command:");
  for (size_t i = 0; i < argc; i++) {
    eprintf(" %s", argv[i]);
  }
  eprintf("\n");

  int status = run_command(argv);
  free(argv);
  if (status != 0) {
    eprintf("Failed to link using system C compiler. Exit status: %d\n",
            status);
    return false;
  }
  return true;
}
