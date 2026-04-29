#pragma once

#include "ast.h"

typedef struct {
  Span span;
  char *message;
} AnalyzeError;

typedef struct {
  AnalyzeError *data;
  size_t length;
} AnalyzeErrorArray;

bool analyze(AST *ast, AnalyzeErrorArray *error_data);

void print_analyze_errors(const char* source, AnalyzeErrorArray type_errors);
void free_analyze_errors(AnalyzeErrorArray *type_errors);
