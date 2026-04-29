#pragma once

#include "ast.h"

Value evaluate_expr(ASTNode *node);
void evaluate_module(ASTNode *node);


