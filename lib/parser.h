#pragma once

#include "ast.h"
#include "lexer.h"

#define MAX_NUM_EXPECTED 20

typedef struct {
  // Index of the first unparsed token.
  size_t first_unparsed_token;
  // The kinds that would have been accepted for the next token.
  TokenKind expected[MAX_NUM_EXPECTED];
  size_t num_expected;
} ParseError;

bool parse_token_stream(TokenStream stream, AST *out_ast,
                        ParseError *error_data);

void print_parse_error(const char* source, TokenStream stream, ParseError error_data);
