#pragma once

#include <llvm-c/Types.h>

#include "ast.h"

LLVMModuleRef generate_llvm(LLVMContextRef ctx, AST *ast) __attribute__((nonnull));
