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

bool analyze(ASTNode* node_array, TypeErrorArray* error_data);
