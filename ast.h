#pragma once

#include "lexer.h"

typedef struct {
  size_t offset;
  size_t length;
} Span;

typedef enum {
  INVALID = 0,
  NAME,
  INTEGER,
  STRING,
  UNARY,
  BINARY,
  FUNCTION_CALL,
  FUNCTION,
  PARAM,
  FOR_LOOP,
  ASSIGN,
  RETURN_STMT,
  DECLARATION, // variable declaration
  MODULE,
} ASTNodeKind;

typedef struct {
  const char *text;
  size_t length;
} Name;

typedef struct ASTNode ASTNode;

typedef struct {
  ListAllocator allocator;
  ASTNode *head;
} AST;

typedef enum {
  tyVOID = 0,
  tyINT,
  tySTRING,
  tyFUNCTION,
  tySTRUCT,
  tyUNION,
  tyALIAS,
  tyTYPE,
} TypeKind;

typedef struct Type Type;
typedef struct Type {
  TypeKind kind;
  union {
    Name name;
    struct {
      Type *params;
      size_t num_params;
      Type *_return;
    } function;
  };
} Type;

typedef struct ASTNode {
  Span span;
  // Next sibling in case of an array.
  ASTNode *next_sibling;
  bool is_analyzed;
  Type type;
  ASTNodeKind kind;
  union {
    Name name;
    struct {
      const char *text;
      size_t length;
    } integer;
    struct {
      const char *text;
      size_t length;
    } string;
    struct {
      TokenKind op;
      ASTNode *expr;
    } unary;
    struct {
      TokenKind op;
      bool has_parens;
      ASTNode *left;
      ASTNode *right;
    } binary;
    struct {
      ASTNode *function;
      ASTNode *args;
    } function_call;
    struct {
      ASTNode *name;
      ASTNode *type;
    } param;
    struct {
      ASTNode *params;
      size_t num_params;      
      ASTNode *return_type; // nullable
      ASTNode *stmts;
    } function;
    struct {
      ASTNode *init;
      ASTNode *cond;
      ASTNode *step;
      ASTNode *stmts;
    } for_loop;
    struct {
      TokenKind op;
      ASTNode *name;
      ASTNode *value;
    } assign;
    struct {
      ASTNode *expr;
    } return_stmt;
    struct {
      bool is_constant;
      bool is_declared;
      ASTNode *name;
      ASTNode *value;
    } declaration;
    struct {
      ASTNode *declarations;
    } module;
  };
} ASTNode;

Span token_span(Token token);
Span join_spans(Span left, Span right);

bool name_eq(Name left, Name right);
bool type_eq(Type left, Type right);

void free_ast(AST *ast);

void ast_visit(ASTNode *node, size_t depth,
               void(callback)(ASTNode *node, size_t depth));
void ast_print_nodes(ASTNode *node);
const char *ast_node_name(ASTNodeKind kind);
