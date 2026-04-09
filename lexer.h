#pragma once

#include "util.h"

typedef enum {
  WHITESPACE,
  COMMENT,
  // words
  IDENT,
  NUMBER,
  STRING,
  // symbols
  LPAREN,
  RPAREN,
  LBRACE,
  RBRACE,
  LBRACKET,
  RBRACKET,
  DOT,
  PLUS,
  MINUS,
  STAR,
  SLASH,
  HASHTAG,
  SEMI,
  COMMA,
  // keywords
  STRUCT,
  FUNC,
  UNION,
  ENUM,
} TokenKind;

typedef struct {
  size_t line;
  size_t column;
  const char *text;
  size_t length;
  TokenKind kind;
} Token;

void lexer_init(void);
void lexer_free(void);
const char *lexer_token_name(TokenKind kind);
bool lex(const char *source, size_t *offset, Token *out);
