#pragma once

#include "ast.h"

typedef struct {
  // One-based line number.
  size_t line;
  // Zero-based column number.
  size_t column;
  // Length of the symbol this error is about..
  size_t length;  
  char* message;
} CompileError;

typedef struct {
  CompileError *data;
  size_t length;
} CompileErrors;

bool compile(const char *source, AST *out_ast, CompileErrors *out_errors);

CompileErrors diagnose(const char* source);

void free_compile_error(CompileError *error);
void free_compile_errors(CompileErrors* errors);
