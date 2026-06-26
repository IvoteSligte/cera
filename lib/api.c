#include "api.h"
#include "analyzer.h"
#include "lexer.h"
#include "parser.h"

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

bool compile(const char *source, AST *out_ast, Errors *out_errors) {
  *out_ast = (AST){0};
  TokenStream stream = {0};
  LexError lex_error = {0};
  ParseError parse_error = {0};
  AnalyzeErrorArray analyze_errors = {0};

  // Error message info
  char *message = NULL;
  size_t line = 0;
  size_t column = 0;
  size_t length = 0;

#ifdef DEBUG_EVALUATOR
  evaluator_source = source;
#endif
  if (!(fill_token_stream(source, &stream, &lex_error))) {
    get_lex_error_info(lex_error, &message, &line, &column);
    push_error(out_errors, new_error(message, line, column, 1));

    free_token_stream(&stream);
    return false;
  }
  if (!parse_token_stream(stream, out_ast, &parse_error)) {
    get_parse_error_info(source, stream, parse_error, &message, &line, &column,
                         &length);
    push_error(out_errors, new_error(message, line, column, length));

    free_token_stream(&stream);
    free_ast(out_ast);
    return false;
  }
  if (!analyze(out_ast, &analyze_errors)) {
    for (size_t i = 0; i < analyze_errors.length; i++) {
      AnalyzeError error = analyze_errors.data[i];
      get_analyze_error_info(source, error, &message, &line, &column, &length);
      push_error(out_errors, new_error(message, line, column, length));
    }
    free_analyze_errors(&analyze_errors);
    free_token_stream(&stream);
    free_ast(out_ast);
    return false;
  }
  free_token_stream(&stream);
  return true;
}

Errors diagnose(const char *source) {
  Errors errors = {0};
  AST ast = {0};
  compile(source, &ast, &errors);
  free_ast(&ast);
  return errors;
}

void free_compile_error(Error *error) {
  free(error->message);
  *error = (Error){0};
}

void free_compile_errors(Errors *errors) {
  for (size_t i = 0; i < errors->length; i++)
    free_compile_error(&errors->data[i]);
  free(errors->data);
  *errors = (Errors){0};
}
