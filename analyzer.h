#pragma once

#include "ast.h"

typedef struct {
  Span span;  
  char *message;
} TypeError;
