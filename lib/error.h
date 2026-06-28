#pragma once

#include <stddef.h>

typedef struct CompileError {
  // Offset in characters from the start of the file.
  size_t offset;
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

void print_compile_error(const char* source, CompileError error);
void print_compile_errors(const char* source, CompileErrors errors);

void free_compile_error(CompileError *error);
void free_compile_errors(CompileErrors* errors);

