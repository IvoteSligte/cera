#pragma once

#include "lexer.h"

typedef struct {
  size_t offset;
  size_t length;
} Span;

typedef struct ASTNode ASTNode;

typedef struct ASTNode {
  Span span;
  enum {
    NAME,
    INTEGER,
    STRING,
    UNARY,
    BINARY,
    FUNCTION,
    PARAM,
    FOR_LOOP,
    ASSIGN,
    DECLARATION, // variable declaration
    MODULE,
  } kind;
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
      ASTNode *expr;
    } unary;
    struct {
      TokenKind op;
      ASTNode *left;
      ASTNode* right;            
    } binary;
    struct {
      ASTNode *name;
      ASTNode *type;
    } param;
    struct {
      ASTNode *name;
      ASTNode *params;
      size_t num_params;
      ASTNode* returnType;
      ASTNode *stmts;
      size_t num_stmts;
    } function;
    struct {
      ASTNode *init;
      ASTNode *cond;
      ASTNode *step;
      ASTNode *stmts;
      size_t num_stmts;
    } for_loop;
    struct {
      TokenKind op;      
      ASTNode *name;
      ASTNode *value;
    } assign;
    struct {
      ASTNode *name;
      ASTNode *value;
    } declaration;
    struct {
      ASTNode *definitions;
      size_t num_definitions;
    } module;
  };
} ASTNode;

Span token_span(Token token);
Span join_spans(Span left, Span right);
