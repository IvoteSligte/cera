#pragma once

#include "ast.h"

#ifdef DEBUG_EVALUATOR
// must be set to use the DEBUG_EVALUATOR flag
extern const char *evaluator_source;
#endif

void evaluate_expr(ASTNode *node, size_t recursion_depth, Value* stack_frame, Value* out);
void evaluate_module(ASTNode *node);


