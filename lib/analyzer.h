#pragma once

#include "ast.h"
#include "llvm.h"

typedef struct {
  Span span;
  char *message;
} AnalyzeError;

typedef struct {
  AnalyzeError *data;
  size_t length;
} AnalyzeErrorArray;

bool analyze(AST *ast, AnalyzeErrorArray *error_data);

void print_analyze_errors(const char *source, AnalyzeErrorArray errors);

void get_analyze_error_info(const char *source, AnalyzeError error,
                            char **out_message, size_t *out_line,
                            size_t *out_column, size_t *out_length);

void free_analyze_errors(AnalyzeErrorArray *errors);
