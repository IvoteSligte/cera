#include <llvm-c/Analysis.h>
#include <llvm-c/Core.h>
#include <llvm-c/IRReader.h>
#include <llvm-c/Linker.h>
#include <llvm-c/Support.h>
#include <llvm-c/Target.h>

#include "ast_macro.h"
#include "llvm.h"

#define FAIL($prefix)                                                          \
  *out_error = ssprintf($prefix ": %s", *out_error);                           \
  free(error);                                                                 \
  return false

// TODO: keep track of initialization with boolean?
//       does LLVM guarantee that calling LLVMInitialize*()
//       is a no-op after the first time?
void llvm_init(void) {
  // LLVM uses 0 = success here for some reason
  if (LLVMInitializeNativeTarget()) {
    panicf("Failed to initialize LLVM native target.");
  }
  if (LLVMInitializeNativeAsmPrinter()) {
    panicf("Failed to initialize LLVM native ASM printer.");
  };
  if (LLVMInitializeNativeAsmParser()) {
    panicf("Failed to initialize LLVM native ASM parser.");
  }
}

LLVMTypeRef llvm_declare_struct(LLVMContextRef ctx, ASTNode *decl_node) {
  assert(decl_node->kind == aSTRUCT_DECL);
  auto decl = &decl_node->struct_decl;
  if (decl->llvm_type != NULL) {
    return decl->llvm_type;
  }
  char *name =
      strndup(decl->name->name.name.text, decl->name->name.name.length);
  decl->llvm_type = LLVMStructCreateNamed(ctx, name);

  free(name);
  return decl->llvm_type;
}

LLVMTypeRef to_llvm_type(LLVMContextRef ctx, LLVMPrimitives prim, Type type) {
  switch (type.kind) {
  case tyUNKNOWN:
    panicf("called to_llvm_type on UNKNOWN type");
  case tyVOID:
    return prim._void;
  case tyI8:
  case tyU8:
    return prim.i8;
  case tyI16:
  case tyU16:
    return prim.i16;
  case tyI32:
  case tyU32:
    return prim.i32;
  case tyI64:
  case tyU64:
    return prim.i64;
  case tyUINT:
  case tyINT:
    return prim._int;
  case tyBOOL:
    return prim._bool;
  case tySTRING:
    return prim.string;
  case tyOPAQUE_PTR:
    return prim.ptr;
  case tyPTR:
    return LLVMPointerType(to_llvm_type(ctx, prim, *type.pointee_type), 0);
  case tyFUNCTION: {
    auto function = type.function;
    size_t num_params = function.params.length;
    LLVMTypeRef params[MAX(num_params, 1)];
    ITER_ARRAY(function.params, param,
               { params[i] = to_llvm_type(ctx, prim, param); });
    return LLVMFunctionType(to_llvm_type(ctx, prim, *function._return), params,
                            num_params, false);
  }
  case tySTRUCT: {
    assert(type._struct->kind == aSTRUCT_DECL);
    auto decl = &type._struct->struct_decl;
    if (decl->llvm_type != NULL) {
      return decl->llvm_type;
    }
    llvm_declare_struct(ctx, type._struct);
    break;
  }
  case tyUNION:
    panicf("unimplemented: to_llvm_type for unions");
  case tyTYPE:
    panicf("unreachable");
    break;
  }
  panicf("not a type. kind int: %d", type.kind);
}
