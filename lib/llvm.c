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

LLVMState llvm_create_state(void) {
  return (LLVMState){.ctx = LLVMContextCreate()};
}

void llvm_destroy_state(LLVMState *state) {
  LLVMContextDispose(state->ctx);
  *state = (LLVMState){0};
}

bool llvm_load_module(RandomAllocator *allocator, LLVMState *state,
                      ExternMod *extern_mod, String path, char **out_error) {
  LLVMModuleRef mod = NULL;
  char *path_string = strndup(path.text, path.length);
  char *error = NULL;

  LLVMMemoryBufferRef membuf = NULL;
  if (LLVMCreateMemoryBufferWithContentsOfFile(path_string, &membuf,
                                               out_error)) {
    free(path_string);
    FAIL("Failed to read LLVM file");
  }
  free(path_string);

  if (LLVMParseIRInContext(state->ctx, membuf, &mod, out_error)) {
    FAIL("Failed to parse LLVM IR");
  }

  if (LLVMVerifyModule(mod, LLVMReturnStatusAction, out_error)) {
    FAIL("Failed to verify LLVM module");
  } else {
    eprintf("Verified LLVM module.\n");
  }
  llvm_fill_extern_mod(allocator, extern_mod, mod);

  if (state->extern_mod == NULL) {
    state->extern_mod = mod;
  } else {
    // false = success
    assert(!LLVMLinkModules2(state->extern_mod, mod));
  }

  size_t ident_length = 0;
  const char *ident_string = LLVMGetModuleIdentifier(mod, &ident_length);
  eprintf("Successfully imported LLVM module: `%.*s`\n", (int)ident_length,
          ident_string);
  return true;
}

void llvm_link_into(LLVMState *state, LLVMModuleRef dst) {
  if (state->extern_mod != NULL) {
    assert(!LLVMLinkModules2(dst, state->extern_mod)); // false = success
    state->extern_mod = NULL; // module is consumed, clear dangling pointer
  }
}

LLVMTypeRef llvm_declare_struct(LLVMState *state, ASTNode *decl_node) {
  assert(decl_node->kind == aSTRUCT_DECL);
  auto decl = &decl_node->struct_decl;
  if (decl->llvm_type != NULL) {
    return decl->llvm_type;
  }
  char *name =
      strndup(decl->name->name.name.text, decl->name->name.name.length);
  decl->llvm_type = LLVMStructCreateNamed(state->ctx, name);

  free(name);
  return decl->llvm_type;
}

LLVMTypeRef to_llvm_type(LLVMState *state, LLVMPrimitives prim, Type type) {
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
  case tyPTR:
    return LLVMPointerType(to_llvm_type(state, prim, *type.pointee_type), 0);
  case tyFUNCTION: {
    auto function = type.function;
    size_t num_params = function.params.length;
    LLVMTypeRef params[MAX(num_params, 1)];
    ITER_ARRAY(function.params, param,
               { params[i] = to_llvm_type(state, prim, param); });
    return LLVMFunctionType(to_llvm_type(state, prim, *function._return),
                            params, num_params, false);
  }
  case tySTRUCT: {
    assert(type._struct->kind == aSTRUCT_DECL);
    auto decl = &type._struct->struct_decl;
    if (decl->llvm_type != NULL) {
      return decl->llvm_type;
    }
    llvm_declare_struct(state, type._struct);
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

Type from_llvm_type(RandomAllocator *allocator, LLVMTypeRef llvm_type) {
  auto kind = LLVMGetTypeKind(llvm_type);
  switch (kind) {
  case LLVMVoidTypeKind:
    return (Type){.kind = tyVOID};
  case LLVMStructTypeKind:
    panicf("unimplemented: from_llvm_type for struct");
    /* return (Type){.kind = tySTRUCT, ._struct = }; */
  case LLVMPointerTypeKind: {
    return (Type){.kind = tyOPAQUE_PTR};
  }
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
    panicf("unimplemented: integer types other than 1 and 64 bits");
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
      type.function.params.data[i] = FROM_LLVM_TYPE(llvm_param_types[i]);
    }
    *type.function._return = FROM_LLVM_TYPE(LLVMGetReturnType(llvm_type));
    return type;
  }
  case LLVMArrayTypeKind:
  case LLVMDoubleTypeKind:
  case LLVMFloatTypeKind:
    panicf("unimplemented: from_llvm_type for array,double,float");
  default:
    panicf("unsupported LLVM type kind %d", kind);
  }
}

typedef RandomAllocator Allocator;

static void add_decl(Allocator *allocator, ExternMod *mod, ExternDecl decl) {
  auto array = &mod->decls;
  array->data = ra_recalloc(allocator, array->data,
                            sizeof(ExternDecl) * (array->length + 1));
  array->data[array->length] = decl;
  array->length += 1;
}

void llvm_fill_extern_mod(Allocator *allocator, ExternMod *extern_mod,
                          LLVMModuleRef llvm_mod) {
  for (auto fn = LLVMGetFirstFunction(llvm_mod); fn != NULL;
       fn = LLVMGetNextFunction(fn)) {
    auto type = from_llvm_type(allocator, LLVMGlobalGetValueType(fn));
    Name name = {0};
    name.text = LLVMGetValueName2(fn, &name.length);
    add_decl(allocator, extern_mod, (ExternDecl){.name = name, .type = type});
  }
  for (auto var = LLVMGetFirstGlobal(llvm_mod); var != NULL;
       var = LLVMGetNextGlobal(var)) {
    auto type = from_llvm_type(allocator, LLVMTypeOf(var));
    Name name = {0};
    name.text = LLVMGetValueName2(var, &name.length);
    add_decl(allocator, extern_mod, (ExternDecl){.name = name, .type = type});
  }

  // TODO: Aliases and IFuncs?
}
