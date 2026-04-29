#pragma once

#include "ast.h"

typedef struct {
  Span span;
  char *message;
} TypeError;

typedef struct {
  TypeError *data;
  size_t length;
} TypeErrorArray;

bool analyze(AST *ast, TypeErrorArray *error_data);

void print_analyze_errors(const char* source, TypeErrorArray type_errors);
void free_analyze_errors(TypeErrorArray *type_errors);
