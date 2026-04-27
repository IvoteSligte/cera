#pragma once

#include "util.h"
#include "alloc.h"

typedef enum {
  tWHITESPACE,
  tCOMMENT,
  // words
  tIDENT,
  tNUMBER,
  tSTRING,
  // delimiters
  tLPAREN,
  tRPAREN,
  tLBRACE,
  tRBRACE,
  tLBRACKET,
  tRBRACKET,
  // operators
  tPLUS,
  tMINUS,
  tSTAR,
  tSLASH,
  // misc symbols
  tHASHTAG,
  tSEMI,
  tCOMMA,
  tDOT,
  tEQ,
  tCOL,
  tCOLEQ,
  tCOLCOL,
  // keywords
  tSTRUCT,
  tUNION,
  tENUM,
  tRETURN,
} TokenKind;

typedef struct {
  size_t offset;
  const char *text;
  size_t length;
  TokenKind kind;
} Token;

typedef enum {
  LEX_OK = 0,
  LEX_NO_MATCH,
  LEX_EOF,
} LexResult;

void lexer_init(void);
void lexer_free(void);
const char *token_name(TokenKind kind);
const char *token_display_name(TokenKind kind);
int token_precedence(TokenKind kind);
LexResult lex(const char *source, size_t *offset, Token *out);

void lexer_print_tokens(const char *source);

typedef struct {
  Token *data;
  size_t length;
} TokenStream;

void lexer_print_token_stream(TokenStream stream);
bool fill_token_stream(const char *source, TokenStream *out);
void free_token_stream(TokenStream stream);
bool peek_token(TokenStream stream, size_t token_index, Token *out);
