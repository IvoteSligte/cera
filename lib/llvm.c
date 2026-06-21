#include <llvm-c/Analysis.h>
#include <llvm-c/Core.h>
#include <llvm-c/IRReader.h>
#include <llvm-c/Linker.h>
#include <llvm-c/Support.h>
#include <llvm-c/Target.h>

#include "ast_macro.h"
#include "demangle.h"
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
  eprintf("Importing LLVM module `%.*s`.\n", FMT(path));

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
    eprintf("Linking LLVM imports.\n");
    assert(!LLVMLinkModules2(dst, state->extern_mod)); // false = success
    state->extern_mod = NULL; // module is consumed, clear dangling pointer
    eprintf("Finished linking LLVM imports.\n");
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
    return prim.ptr;
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
    Name mangled_name = {0};
    mangled_name.text = LLVMGetValueName2(fn, &mangled_name.length);
    Name name = demangle(allocator, mangled_name);
    auto type = from_llvm_type(allocator, LLVMGlobalGetValueType(fn));
    if (type.kind == tyUNKNOWN) {
      eprintf("unknown type in LLVM declaration %.*s. skipping.\n", FMT(name));
      continue;
    }
    add_decl(
        allocator, extern_mod,
        (ExternDecl){.name = name, .mangled_name = mangled_name, .type = type});
  }
  // TODO:
  /* for (auto var = LLVMGetFirstGlobal(llvm_mod); var != NULL; */
  /*      var = LLVMGetNextGlobal(var)) { */
  /*   Name name = {0}; */
  /*   name.text = LLVMGetValueName2(var, &name.length); */
  /*   auto type = from_llvm_type(allocator, LLVMTypeOf(var)); */
  /*   if (type.kind == tyUNKNOWN) { */
  /*     eprintf("unknown type in LLVM declaration %.*s. skipping.\n",
   * FMT(name)); */
  /*     continue; */
  /*   } */
  /*   add_decl(allocator, extern_mod, (ExternDecl){.name = name, .type =
   * type}); */
  /* } */

  // TODO: Aliases and IFuncs?
}
