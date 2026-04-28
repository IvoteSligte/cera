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

typedef struct {
  const char *source;
  size_t offset;
} LexError;

// Initializes lexer globals. MUST be called before lex().
void lexer_init(void);
// Frees lexer globals.
void lexer_free(void);

const char *token_name(TokenKind kind);
const char *token_display_name(TokenKind kind);
int token_precedence(TokenKind kind);

LexResult lex(const char *source, size_t *offset, Token *out, LexError* error_data);

void print_lex_error(LexError error);

typedef struct {
  Token *data;
  size_t length;
} TokenStream;

bool fill_token_stream(const char* source, TokenStream *out, LexError* error_data);
void free_token_stream(TokenStream* stream);
void print_token_stream(TokenStream stream);
bool peek_token(TokenStream stream, size_t token_index, Token *out);
