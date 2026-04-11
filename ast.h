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
  DECLARATION, // variable declaration
  MODULE,
} ASTNodeKind;

typedef struct ASTNode ASTNode;

typedef struct ASTNode {
  Span span;
  // The number of nodes in this AST. At least 1.
  size_t tree_size;
  ASTNodeKind kind;
  union {
    struct {
      const char *text;
      size_t length;
    } name;
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
      size_t expr;
    } unary;
    struct {
      TokenKind op;
      bool has_parens;
      size_t left;
      size_t right;
    } binary;
    struct {
      size_t name;
      size_t args;
      size_t num_args;
    } function_call;
    struct {
      size_t name;
      size_t type;
    } param;
    struct {
      size_t name;
      size_t params;
      size_t num_params;
      size_t return_type;
      bool has_return_type;
      size_t stmts;
      size_t num_stmts;
    } function;
    struct {
      size_t init;
      size_t cond;
      size_t step;
      size_t stmts;
      size_t num_stmts;
    } for_loop;
    struct {
      TokenKind op;
      size_t name;
      size_t value;
    } assign;
    struct {
      size_t name;
      size_t value;
    } declaration;
    struct {
      size_t definitions;
      size_t num_definitions;
    } module;
  };
} ASTNode;

Span token_span(Token token);
Span join_spans(Span left, Span right);

void free_ast(ASTNode *node_array);

void ast_visit(ASTNode *node_array, size_t index, size_t depth,
               void(callback)(ASTNode *node_array, size_t index, size_t depth));
void ast_print_nodes(ASTNode *node_array, size_t index);
const char* ast_node_name(ASTNodeKind kind);
