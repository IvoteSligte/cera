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
  }
  panicf("not a type. kind int: %d", type.kind);
}

#define ALLOC_TYPE ra_calloc(allocator, sizeof(Type))
#define ALLOC_TYPE_ARRAY($size)                                                \
  (TypeArray) {                                                                \
    .data = ra_calloc(allocator, ($size) * sizeof(Type)), .length = ($size)    \
  }

#define FROM_LLVM_TYPE($type) from_llvm_type(allocator, $type)

static Type UNKNOWN_TYPE = {.kind = tyUNKNOWN};

Type from_llvm_type(RandomAllocator *allocator, LLVMTypeRef llvm_type) {
  auto kind = LLVMGetTypeKind(llvm_type);
  switch (kind) {
  case LLVMVoidTypeKind:
    return (Type){.kind = tyVOID};
  case LLVMStructTypeKind:
    eprintf("unimplemented: from_llvm_type for struct\n");
    return UNKNOWN_TYPE;
  case LLVMPointerTypeKind:
    return (Type){.kind = tyOPAQUE_PTR};
  case LLVMIntegerTypeKind: {
    size_t width = LLVMGetIntTypeWidth(llvm_type);
    if (width == 1) {
      return (Type){.kind = tyBOOL};
    }
    // NOTE: int types can differ in size depending on hardware,
    // so a type should be added that allows `i64 == int` or `i32 == int`
    // depending on the current compilation target's integer width
    if (width == 64) {
      return (Type){.kind = tyINT};
    }
    eprintf("unimplemented: integer types other than 1 and 64 bits. found %zu "
            "bit integer\n",
            width);
    return UNKNOWN_TYPE;
  }
  case LLVMFunctionTypeKind: {
    size_t param_count = LLVMCountParamTypes(llvm_type);
    LLVMTypeRef llvm_param_types[MAX(1, param_count)];
    LLVMGetParamTypes(llvm_type, llvm_param_types);
    Type type = (Type){.kind = tyFUNCTION,
                       .function = {
                           .params = ALLOC_TYPE_ARRAY(param_count),
                           ._return = ALLOC_TYPE,
                       }};
    for (size_t i = 0; i < param_count; i++) {
      Type param_type = FROM_LLVM_TYPE(llvm_param_types[i]);
      if (param_type.kind == tyUNKNOWN) {
        return UNKNOWN_TYPE;
      }
      type.function.params.data[i] = param_type;
    }
    *type.function._return = FROM_LLVM_TYPE(LLVMGetReturnType(llvm_type));
    if (type.function._return->kind == tyUNKNOWN) {
      return UNKNOWN_TYPE;
    }
    return type;
  }
  case LLVMArrayTypeKind:
  case LLVMDoubleTypeKind:
  case LLVMFloatTypeKind:
    eprintf("unsupported LLVM type kind %d found (one of array,double,float)\n",
            kind);
    return UNKNOWN_TYPE;
  default:
    eprintf("unsupported LLVM type kind %d found\n", kind);
    return UNKNOWN_TYPE;
  }
}
