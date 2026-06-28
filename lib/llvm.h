#pragma once

#include <llvm-c/Core.h>

#include "ast.h"

#undef ARRAY

typedef struct {
  LLVMTypeRef _void;
  // Note that signedness does not matter for the LLVM type.
  LLVMTypeRef i8;
  LLVMTypeRef i16;
  LLVMTypeRef i32;
  LLVMTypeRef i64;
  LLVMTypeRef _int;
  LLVMTypeRef _bool;
  LLVMTypeRef ptr;
  LLVMTypeRef str;
} LLVMPrimitives;

void llvm_init(void);

LLVMTypeRef llvm_declare_struct(LLVMContextRef ctx, ASTNode *decl_node)
    __attribute__((nonnull));

LLVMTypeRef to_llvm_type(LLVMContextRef ctx, LLVMPrimitives prim, Type type)
    __attribute__((nonnull));

// Compiles an LLVM module to an object file, writing it to a file named FILENAME.
void llvm_compile_to_file(LLVMModuleRef mod, const char *filename);
