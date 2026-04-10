#pragma once

#include "lexer.h"
#include "ast.h"

bool parse(TokenStream stream, ASTNode** out);

