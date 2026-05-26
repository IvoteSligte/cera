#pragma once

#include "ast.h"

#ifdef DEBUG_EVALUATOR
// must be set to use the DEBUG_EVALUATOR flag
extern const char *evaluator_source;
#endif

void evaluate_expr(ASTNode *node, size_t recursion_depth, char* stack_frame, char* out);
void evaluate_module(ASTNode *node);


