
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
  size_t line, column;
} Span;

typedef enum {
  uNONE,   // nothing
  uSTRING, // unfinished string

  INTEGER,
  STRING,
  IDENT,
  LBRACE,
  RBRACE,
  LPAREN,
  RPAREN,
  LESS,
  GREATER,
  EQ,
  SEMI,
  STAR,
  SLASH,
  HASHTAG,
  STRUCT,
  UNION,
} TokenKind;

typedef struct {
  TokenKind kind;
  Span span;
  char *text;
} Token;

#define MAX_NUM_TOKENS 1000

Token tokens[MAX_NUM_TOKENS];
size_t num_tokens = 0;

void lex(const char *source) {
  size_t line = 0;
  size_t column = 0;
  size_t left = 0;
  size_t right = 0;
  TokenKind kind = uNONE;
  char c;

#define emit_token                                                             \
  {                                                                            \
    if (num_tokens >= MAX_NUM_TOKENS) {                                        \
      fprintf(stderr, "Number of lexed tokens exceeded the maximum.\n");       \
      abort();                                                                 \
    }                                                                          \
    if (kind != uNONE) {                                                       \
      if (kind == uSTRING) {                                                   \
        fprintf(stderr, "Tried to emit token while lexing string literal.\n"); \
        abort();                                                               \
      }                                                                        \
      tokens[num_tokens] = (Token){                                            \
          .kind = kind,                                                        \
          .span = {line, column},                                              \
          .text = strndup(&source[left], right - left),                        \
      };                                                                       \
      num_tokens++;                                                            \
      kind = uNONE;                                                            \
    }                                                                          \
    left = right + 1;                                                          \
  }

  for (;;) {
    c = source[right];
    switch (c) {
    case EOF:
      emit_token;
      return;
    case ' ':
    case '\t':
      if (kind != uSTRING) {
        emit_token;
      }
      right++;
      continue;
    case '\n':
      if (kind != uSTRING) {
        emit_token;
      }
      right++;      
      continue;
    case '"':
      if (kind == uSTRING) {
        // finished lexing string literal
        right++;
        kind = STRING;
        emit_token;
      } else {
        emit_token;
        right++;
      }
      continue;
    case '/':
      if (source[right + 1] == '/') {
        // line comment
        left = ;
        continue
      } else {
        right++;
        kind = SLASH;
        emit_token;
      }
      if (kind == SLASH) {

        
        kind = uNONE;
      } else {
        kind = SLASH;
        right++;
      }
      continue;
    }
    if (c >= '0' && c <= '9') {
      if ()
        right++;
      kind = INTEGER;
      continue;
    }
    if (c >= '')
      ;
  }
}
