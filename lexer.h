#pragma once

#include "util.h"

typedef enum {
  tWHITESPACE,
  tCOMMENT,
  // words
  tIDENT,
  tNUMBER,
  tSTRING,
  // symbols
  tLPAREN,
  tRPAREN,
  tLBRACE,
  tRBRACE,
  tLBRACKET,
  tRBRACKET,
  tDOT,
  tPLUS,
  tMINUS,
  tSTAR,
  tSLASH,
  tHASHTAG,
  tSEMI,
  tCOMMA,
  tEQ,
  tCOLONEQ,
  // keywords
  tSTRUCT,
  tFUNC,
  tUNION,
  tENUM,
} TokenKind;

typedef struct {
  size_t offset;
  const char *text;
  size_t length;  
  TokenKind kind;
} Token;

void lexer_init(void);
void lexer_free(void);
const char *lexer_token_name(TokenKind kind);
bool lex(const char *source, size_t *offset, Token *out);

typedef struct {
  Token *data;
  size_t end;
} TokenStream;

bool peek_token(TokenStream stream, size_t token_index, Token *out);
