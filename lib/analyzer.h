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
                            CompileError *out);

void free_analyze_errors(AnalyzeErrorArray *errors);
