#pragma once

#include "ast.h"

Value evaluate_expr(ASTNode *node, size_t recursion_depth);
void evaluate_module(ASTNode *node);


