#pragma once

#include "ast.h"

void evaluate_expr(ASTNode *node, size_t recursion_depth, Value* stack_frame, Value* out);
void evaluate_module(ASTNode *node);


