#include "api.h"
#include "analyzer.h"
#include "generator.h"
#include "lexer.h"
#include "llvm.h"
#include "parser.h"

#ifdef __linux__
#include <sys/wait.h>
#include <unistd.h>
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

Errors compile_and_run(const char *source) {
  Errors errors = {0};
  AST ast = {0};
  if (parse_and_analyze(source, &ast, &errors)) {
    LLVMContextRef ctx = LLVMContextCreate();
    LLVMModuleRef mod = generate_llvm(ctx, &ast);
    llvm_run(mod); // takes ownership of the module
    LLVMContextDispose(ctx);
  }
  free_ast(&ast);
  return errors;
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

bool link_to_binary(const char *object_file, const char *output_file) {
  eprintf("Linking to binary.\n");
  assert(!is_empty_string(object_file));
  assert(!is_empty_string(output_file));

  // TODO: cross-platform way to call system C compiler
  pid_t pid = fork();
  if (pid == 0) {
    char *const argv[] = {
        "cc", STARTUP_PATH,        BUILTIN_PATH, strdup(object_file),
        "-o", strdup(output_file), NULL};
    execvp("cc", argv); // can fail if cc is not in PATH
    pprintf("Failed to link to binary using system C compiler.");
    exit(1);
  }
  int status = 0;
  if (waitpid(pid, &status, 0) == -1) {
    panicf("Failed to wait for system C compiler to finish linking.");
  }
  if (WEXITSTATUS(status) != 0) {
    if (WIFSIGNALED(status)) {
      eprintf("Failed to link to binary. System C compiler was terminated by "
              "signal: %d\n",
              WTERMSIG(status));
    } else {
      eprintf("Failed to link to binary. System C compiler exit status: %d\n",
              WEXITSTATUS(status));
    }
    return false;
  }
  eprintf("Successfully linked to binary.\n");
  return true;
}
