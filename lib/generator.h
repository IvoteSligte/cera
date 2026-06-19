#pragma once

#include <llvm-c/Types.h>

#include "ast.h"
#include "llvm.h"

void generate_and_evaluate(LLVMState *llvm_state, AST *ast)
    __attribute__((nonnull));
