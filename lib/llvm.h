#pragma once

#include <llvm-c/Core.h>

#include "ast.h"

typedef struct {
  LLVMContextRef ctx;
  // Composite of all imported LLVM modules.
  LLVMModuleRef extern_mod;
} LLVMState;

#undef ARRAY

typedef struct {
  LLVMTypeRef _void;
  LLVMTypeRef _int;
  LLVMTypeRef _bool;
  LLVMTypeRef ptr;
  LLVMTypeRef string;
} LLVMPrimitives;

void llvm_init(void);

LLVMState llvm_create_state(void);
void llvm_destroy_state(LLVMState *state);

// Loads a module from a file, setting out_error and returning false on error.
// Adds the loaded module to state.
// Adds declarations in the module to extern_mod.
bool llvm_load_module(RandomAllocator *allocator, LLVMState *state,
                      ExternMod *extern_mod, String path, char **out_error)
    __attribute__((nonnull));

// Links the module in the state into the destination module.
// This consumes the module in the state.
void llvm_link_into(LLVMState *state, LLVMModuleRef dst)
    __attribute__((nonnull));

LLVMTypeRef llvm_declare_struct(LLVMState *state, ASTNode *decl_node)
    __attribute__((nonnull));

LLVMTypeRef to_llvm_type(LLVMState *state, LLVMPrimitives prim, Type type)
    __attribute__((nonnull));
Type from_llvm_type(RandomAllocator *allocator, LLVMTypeRef llvm_type)
    __attribute__((nonnull));

// Fills extern_mod with declarations in llvm_mod.
void llvm_fill_extern_mod(RandomAllocator *allocator, ExternMod *extern_mod,
                          LLVMModuleRef llvm_mod) __attribute__((nonnull));
