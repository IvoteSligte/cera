#include <llvm-c/TargetMachine.h>

#include "ast_macro.h"
#include "llvm.h"

#define FAIL($prefix)                                                          \
  *out_error = ssprintf($prefix ": %s", *out_error);                           \
  free(error);                                                                 \
  return false

_Atomic bool llvm_is_init = false;

// TODO: keep track of initialization with boolean?
//       does LLVM guarantee that calling LLVMInitialize*()
//       is a no-op after the first time?
void llvm_init(void) {
  if (llvm_is_init) {
    return;
  }
  llvm_is_init = true;
  // LLVM uses 0 = success here for some reason
  if (LLVMInitializeNativeTarget()) {
    panicf("Failed to initialize LLVM native target.");
  }
  if (LLVMInitializeNativeAsmPrinter()) {
    panicf("Failed to initialize LLVM native ASM printer.");
  };
  // NOTE: is the parser needed?
  if (LLVMInitializeNativeAsmParser()) {
    panicf("Failed to initialize LLVM native ASM parser.");
  }
  LLVMInitializeAllTargetInfos(); // no Native equivalent for this
  if (LLVMInitializeNativeTarget()) {
    panicf("Failed to initialize LLVM native target.");
  }
  LLVMInitializeAllTargetMCs(); // no Native equivalent for this
  eprintf("Initialized LLVM.\n");
}

LLVMTypeRef llvm_declare_struct(LLVMContextRef ctx, ASTNode *decl_node) {
  assert(decl_node->kind == aSTRUCT_DECL);
  auto decl = &decl_node->struct_decl;
  if (decl->llvm_type != NULL) {
    return decl->llvm_type;
  }
  char *name = name_dup_to_string(decl->name->name.name);
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
  case tyCHAR:
    return prim.i32;
  case tyI64:
  case tyU64:
    return prim.i64;
  case tyUINT:
  case tyINT:
    return prim._int;
  case tyBOOL:
    return prim._bool;
  case tySTR:
    return prim.str;
  case tyPTR:
  case tyARRAY:
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

// TODO: convert these to compiler arguments
#define OPT_LEVEL 0
#define RELOC_MODE LLVMRelocDefault
#define CODE_MODEL LLVMCodeModelDefault

// Creates a target machine and sets the given module's data layout and target
// accordingly.
LLVMTargetMachineRef llvm_create_target_machine(LLVMModuleRef mod) {
  llvm_init();
  char *target_triple = LLVMGetDefaultTargetTriple();
  eprintf("LLVM detected native target triple: %s\n", target_triple);
  LLVMTargetRef target = NULL;
  char *error = NULL;
  if (LLVMGetTargetFromTriple(target_triple, &target, &error)) {
    LLVMDisposeMessage(target_triple);
    panicf("Failed to create LLVM compilation target. Error: %s",
           error == NULL ? "<unknown>" : error);
  }
  char *cpu_name = LLVMGetHostCPUName();
  eprintf("LLVM detected CPU: %s\n", cpu_name);
  char *cpu_features = LLVMGetHostCPUFeatures();
  LLVMDisposeMessage(error);
  auto target_machine =
      LLVMCreateTargetMachine(target, target_triple, cpu_name, cpu_features,
                              OPT_LEVEL, RELOC_MODE, CODE_MODEL);
  auto data_layout = LLVMCreateTargetDataLayout(target_machine);
  LLVMSetModuleDataLayout(mod, data_layout);
  LLVMSetTarget(mod, target_triple);
  LLVMDisposeTargetData(data_layout);

  LLVMDisposeMessage(target_triple);
  LLVMDisposeMessage(cpu_name);
  LLVMDisposeMessage(cpu_features);
  return target_machine;
}

void llvm_compile_to_file(LLVMModuleRef mod, const char *filename) {
  auto target_machine = llvm_create_target_machine(mod);
  char *error = NULL;
  if (LLVMTargetMachineEmitToFile(target_machine, mod, filename, LLVMObjectFile,
                                  &error)) {
    LLVMDisposeTargetMachine(target_machine);
    panicf("Failed to emit machine code to object file. Error: %s", error);
  }
  if (error != NULL)
    LLVMDisposeMessage(error);
  LLVMDisposeTargetMachine(target_machine);
}
