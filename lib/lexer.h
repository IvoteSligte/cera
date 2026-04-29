#pragma once

#include "alloc.h"
#include "util.h"

typedef enum {
  tEOF, // returned by peek_token when the end-of-file is reached
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
  tPLUS_EQ,
  tMINUS_EQ,
  tSTAR_EQ,
  tSLASH_EQ,
  tLT,
  tGT,
  tLT_EQ,
  tGT_EQ,
  tEQ_EQ,
  tAMP_AMP,
  tBAR_BAR,
  // misc symbols
  tHASHTAG,
  tSEMI,
  tCOMMA,
  tDOT,
  tEQ,
  tCOL,
  tCOL_EQ,
  tCOL_COL,
  // keywords
  tSTRUCT,
  tUNION,
  tENUM,
  tRETURN,
  tFOR,
  tIF,
  tWHILE,
  tTRUE,
  tFALSE,
  // metasymbol for the number of token kinds
  tKIND_COUNT,
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

LexResult lex(const char *source, size_t *offset, Token *out,
              LexError *error_data);

void print_lex_error(LexError error);

typedef struct {
  const char *source;
  Token *data;
  size_t length;
} TokenStream;

bool fill_token_stream(const char *source, TokenStream *out,
                       LexError *error_data);
void free_token_stream(TokenStream *stream);
void print_token_stream(TokenStream stream);
Token get_token(TokenStream stream, size_t token_index);
