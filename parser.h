#pragma once

#include "lexer.h"
#include "ast.h"

#define PARSER(suffix) bool parse##suffix(Token* tokens, size_t num_tokens, size_t* offset, ASTNode* out)

PARSER();

