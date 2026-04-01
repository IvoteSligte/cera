#include "lib/basic.h"

typedef struct {
  size_t line;
  size_t column;
  const String *source;
} Span;

typedef struct AST AST;

typedef struct {
  size_t len;
  AST *data;
} ASTArray;

typedef struct AST {
  Span span;
  union {
    String ident;
    String string;
    struct {
      AST *name;
      ASTArray matches;
      AST *to;
    } def;
    struct {
      AST *alias;
      AST *rule;
    } named;
    struct {
      AST *name;
      AST *elements;
    } make;
    struct {
      AST *left;
      AST *right;
    } either;
    struct {
      ASTArray matches;
    } repeat;
    struct {
      ASTArray match;      
    } maybe;
  };
  enum { IDENT, STRING, DEF, NAMED, MAKE, EITHER, REPEAT, MAYBE };
} AST;
