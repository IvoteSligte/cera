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
  LLVMTypeRef string;
} LLVMPrimitives;

void llvm_init(void);

LLVMTypeRef llvm_declare_struct(LLVMContextRef ctx, ASTNode *decl_node)
    __attribute__((nonnull));

LLVMTypeRef to_llvm_type(LLVMContextRef ctx, LLVMPrimitives prim, Type type)
    __attribute__((nonnull));
Type from_llvm_type(RandomAllocator *allocator, LLVMTypeRef llvm_type)
    __attribute__((nonnull));

// Fills extern_mod with declarations in llvm_mod.
void llvm_fill_extern_mod(RandomAllocator *allocator, ExternMod *extern_mod,
                          LLVMModuleRef llvm_mod) __attribute__((nonnull));
