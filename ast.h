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
      size_t expr;
    } unary;
    struct {
      TokenKind op;
      size_t left;
      size_t right;
    } binary;
    struct {
      size_t name;
      size_t type;      
    } param;
    struct {
      size_t name;
      size_t params;
      size_t num_params;
      size_t returnType;
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
