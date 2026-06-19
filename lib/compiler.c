#include "compiler.h"
#include "analyzer.h"
#include "lexer.h"
#include "parser.h"

CompileError new_error(char *message, size_t line, size_t column) {
  return (CompileError){.message = message, .line = line, .column = column};
}

void push_error(CompileErrors *errors, CompileError error) {
  errors->data =
      realloc(errors->data, sizeof(CompileError) * (errors->length + 1));
  errors->data[errors->length] = error;
  errors->length += 1;
}

bool compile(const char *source, AST *out_ast, CompileErrors *out_errors) {
  *out_ast = (AST){0};
  TokenStream stream = {0};
  LexError lex_error = {0};
  ParseError parse_error = {0};
  AnalyzeErrorArray analyze_errors = {0};

  // Error message info
  char *message = NULL;
  size_t line = 0;
  size_t column = 0;

#ifdef DEBUG_EVALUATOR
  evaluator_source = source;
#endif
  if (!(fill_token_stream(source, &stream, &lex_error))) {
    get_lex_error_info(lex_error, &message, &line, &column);
    push_error(out_errors, new_error(message, line, column));

    free_token_stream(&stream);
    return false;
  }
  if (!parse_token_stream(stream, out_ast, &parse_error)) {
    get_parse_error_info(source, stream, parse_error, &message, &line, &column);
    push_error(out_errors, new_error(message, line, column));

    free_token_stream(&stream);
    free_ast(out_ast);
    return false;
  }
  LLVMState llvm_state = llvm_create_state();
  if (!analyze(&llvm_state, out_ast, &analyze_errors)) {
    for (size_t i = 0; i < analyze_errors.length; i++) {
      AnalyzeError error = analyze_errors.data[i];
      get_analyze_error_info(source, error, &message, &line, &column);
      push_error(out_errors, new_error(message, line, column));
    }
    free_analyze_errors(&analyze_errors);
    free_token_stream(&stream);
    free_ast(out_ast);
    llvm_destroy_state(&llvm_state);
    return false;
  }
  free_token_stream(&stream);
  llvm_destroy_state(&llvm_state);
  return true;
}

void free_compile_error(CompileError *error) {
  free(error->message);
  *error = (CompileError){0};
}

void free_compile_errors(CompileErrors *errors) {
  for (size_t i = 0; i < errors->length; i++)
    free_compile_error(&errors->data[i]);
  free(errors->data);
  *errors = (CompileErrors){0};
}
