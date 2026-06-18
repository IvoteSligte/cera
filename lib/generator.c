
#include <llvm-c/Analysis.h>
#include <llvm-c/Core.h>
#include <llvm-c/ExecutionEngine.h>
#include <llvm-c/IRReader.h>
#include <llvm-c/Linker.h>
#include <llvm-c/Support.h>
#include <llvm-c/Target.h>
#include <llvm-c/TargetMachine.h>
#include <llvm-c/Types.h>

#include "ast_macro.h"
#include "generator.h"

typedef ASTNode Node;
typedef struct {
  LLVMTypeRef _void;
  LLVMTypeRef _int;
  LLVMTypeRef _bool;
  LLVMTypeRef ptr;
  LLVMTypeRef string;
} Primitives;

typedef struct State {
  LLVMContextRef ctx;
  LLVMModuleRef mod;
  LLVMBuilderRef builder;
  LLVMValueRef fn;
  struct {
    LLVMValueRef print_bool;
    LLVMValueRef print_int;
    LLVMValueRef print_string;
  } builtin;
  Primitives prim;
} State;

LLVMTypeRef declare_struct(LLVMContextRef ctx, Node *decl_node) {
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

LLVMTypeRef to_llvm_type(LLVMContextRef ctx, Primitives prim, Type type) {
  switch (type.kind) {
  case tyUNKNOWN:
    panicf("unreachable");
  case tyVOID:
    return prim._void;
  case tyINT:
    return prim._int;
  case tyBOOL:
    return prim._bool;
  case tySTRING:
    return prim.string;
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
    declare_struct(ctx, type._struct);
    break;
  }
  case tyUNION:
    panicf("unimplemented");
  case tyTYPE:
    panicf("unreachable");
  }
  panicf("not a type. kind int: %d", type.kind);
}

#define GEN($node, $name)                                                      \
  generate_node(state, $node);                                                 \
  LLVMValueRef $name = $node->llvm_value;                                      \
  UNUSED($name)

#define BIN($op, $instr)                                                       \
  case t##$op:                                                                 \
    node->llvm_value =                                                         \
        LLVMBuild##$instr(builder, left_value, right_value, "");               \
    break;

#define CMP($op, $pred)                                                        \
  case t##$op:                                                                 \
    node->llvm_value =                                                         \
        LLVMBuildICmp(builder, LLVMInt##$pred, left_value, right_value, "");   \
    break;

#define TO_LLVM_TYPE($type) to_llvm_type(state->ctx, state->prim, $type)

LLVMValueRef declare_function(State *state, Node *decl_node) {
  assert(decl_node->kind == aFUNC_DECL);
  if (decl_node->llvm_value != NULL) {
    return decl_node->llvm_value;
  }
  auto decl = &decl_node->func_decl;
  char *name =
      strndup(decl->name->name.name.text, decl->name->name.name.length);
  auto type = TO_LLVM_TYPE(decl_node->type);

  char *type_string = LLVMPrintTypeToString(type);
  eprintf("Generating function decl `%s`: %s\n", name, type_string);
  LLVMDisposeMessage(type_string);

  auto fn = LLVMAddFunction(state->mod, name, type);
  LLVMSetFunctionCallConv(fn, LLVMCCallConv);
  decl_node->llvm_value = fn;
  free(name);
  return fn;
}

void generate_node(State *state, Node *node);

void append_block_stmts(State *state, LLVMBasicBlockRef block,
                        ASTNodeArray stmts, LLVMBasicBlockRef next_block) {
  auto builder = state->builder;
  LLVMPositionBuilderAtEnd(builder, block);
  bool block_terminated = false;
  ITER_ARRAY(stmts, stmt_node, {
    GEN(stmt_node, stmt_value);
    if (stmt_node->kind == aRETURN_STMT) {
      block_terminated = true;
      break; // skip statements after terminator in this block
    }
  });
  if (!block_terminated) {
    LLVMBuildBr(builder, next_block);
  }
}

#define ASS($op, $instr)                                                       \
  case t##$op: {                                                               \
    auto new_value = LLVMBuild##$instr(builder, target_value, expr_value, ""); \
    LLVMBuildStore(builder, new_value, target_ptr);                            \
    break;                                                                     \
  }

void generate_node(State *state, Node *node) {
  auto ctx = state->ctx;
  auto mod = state->mod;
  auto builder = state->builder;
  SWITCH(node, {
    CASE(name, {
      if (name->decl == NULL) {
        panicf("unimplemented: builtins as variables");
      }
      if (name->decl->llvm_value == NULL) {
        panicf("unimplemented: forward declarations");
      }
      char *label = strndup(name->name.text, name->name.length);
      node->llvm_value = LLVMBuildLoad2(builder, TO_LLVM_TYPE(node->type),
                                        name->decl->llvm_value, label);
      free(label);
    });
    CASE(integer, node->llvm_value =
                      LLVMConstInt(state->prim._int, integer->value, true));
    CASE(boolean, node->llvm_value =
                      LLVMConstInt(state->prim._bool, boolean->value, false));
    CASE(string, {
      // FIXME: use LLVMBuildInsertValue for non-constant strings
      char *text = strndup(string->value.text, string->value.length);
      LLVMValueRef zero = LLVMConstInt(LLVMInt32TypeInContext(ctx), 0, 0);
      LLVMValueRef indices[2] = {zero, zero}; // element 0, offset 0
      auto data = LLVMConstStringInContext2(ctx, string->value.text,
                                            string->value.length, true);
      auto data_type = LLVMTypeOf(data);
      auto data_global = LLVMAddGlobal(mod, data_type, "");
      LLVMSetInitializer(data_global, data);

      LLVMValueRef value[2] = {
          LLVMConstGEP2(data_type, data_global, indices, 2),
          LLVMConstInt(state->prim._int, string->value.length, false)};
      free(text);
      node->llvm_value = LLVMConstNamedStruct(state->prim.string, value, 2);
    });
    CASE(unary, {
      GEN(unary->expr, expr_value);
      switch (unary->op) {
      case tBANG:
        node->llvm_value = LLVMBuildNot(builder, expr_value, "");
        break;
      case tMINUS:
        node->llvm_value = LLVMBuildNeg(builder, expr_value, "");
        break;
      default:
        panicf("unknown unary operator: %s", token_name(unary->op));
      }
    });
    CASE(binary, {
      GEN(binary->left, left_value);
      GEN(binary->right, right_value);
      switch (binary->op) {
        BIN(PLUS, Add);
        BIN(MINUS, Sub);
        BIN(STAR, Mul);
        BIN(SLASH, SDiv);
        CMP(LT, SLT);
        CMP(GT, SGT);
        CMP(LT_EQ, SLE);
        CMP(GT_EQ, SGE);
        CMP(EQ_EQ, EQ);
      case tBANG_EQ: {
        auto eq_value =
            LLVMBuildICmp(builder, LLVMIntEQ, left_value, right_value, "");
        node->llvm_value = LLVMBuildNot(builder, eq_value, "");
        break;
      }
        BIN(AMP_AMP, And);
        BIN(BAR_BAR, Or);
      default:
        panicf("unknown binary operator: %s", token_name(binary->op));
      }
    });
    CASE(func_call, {
      auto function = func_call->function;
      assert(function->kind == aNAME);
      LLVMValueRef fn = NULL;
      if (function->name.decl != NULL) {
        fn = declare_function(state, function->name.decl);
      } else {
        switch (function->name.builtin) {
        case bPRINT_BOOL:
          fn = state->builtin.print_bool;
          break;
        case bPRINT_INT:
          fn = state->builtin.print_int;
          break;
        case bPRINT_STRING:
          fn = state->builtin.print_string;
          break;
        default:
          panicf("Unknown builtin %d.", function->name.builtin);
        }
      }
      size_t num_args = func_call->args.length;
      LLVMValueRef args[MAX(num_args, 1)];
      ITER_ARRAY(func_call->args, arg_node, {
        GEN(arg_node, arg_value);
        args[i] = arg_value;
      });
      node->llvm_value = LLVMBuildCall2(builder, TO_LLVM_TYPE(function->type),
                                        fn, args, num_args, "");
    });
    CASE(ptr_create, {
      // only named values can have addresses right now
      assert(ptr_create->expr->kind == aNAME);
      auto var_name = &ptr_create->expr->name;
      assert(var_name->decl != NULL);
      assert(var_name->decl->llvm_value != NULL);
      node->llvm_value = var_name->decl->llvm_value;
    });
    CASE(ptr_deref, {
      GEN(ptr_deref->expr, ptr_value);
      node->llvm_value =
          LLVMBuildLoad2(builder, TO_LLVM_TYPE(node->type), ptr_value, "");
    });
    CASE(ptr_type, { panicf("unreachable"); });
    // Functions are generated through decl so that they can be declared with a
    // name in LLVM.
    CASE(func_decl, {
      auto fn = declare_function(state, node);
      node->llvm_value = fn;
      state->fn = fn;
      auto entry_block = LLVMAppendBasicBlockInContext(ctx, fn, "entry");
      LLVMPositionBuilderAtEnd(builder, entry_block);

      ITER_ARRAY(func_decl->params, param_node, {
        // TODO: only place parameter on stack when it needs an address
        param_node->llvm_value =
            LLVMBuildAlloca(builder, TO_LLVM_TYPE(param_node->type), "");
        LLVMBuildStore(builder, LLVMGetParam(fn, i), param_node->llvm_value);
      });
      ITER_ARRAY(func_decl->stmts, stmt_node, { GEN(stmt_node, stmt_value); });
      if (func_decl->return_type == NULL) {
        LLVMBuildRetVoid(builder);
      }
      state->fn = NULL;
    });
    // Generate does not need to be called in parameters as they are declared by
    // the function.
    CASE(param, { panicf("params should not be generated directly"); });
    CASE(if_stmt, {
      GEN(if_stmt->cond, cond_value);
      auto then = LLVMAppendBasicBlockInContext(ctx, state->fn, "then");
      auto _else = if_stmt->else_stmts.length > 0
                       ? LLVMAppendBasicBlockInContext(ctx, state->fn, "else")
                       : NULL;
      auto end = LLVMAppendBasicBlockInContext(ctx, state->fn, "end");
      LLVMBuildCondBr(builder, cond_value, then, _else == NULL ? end : _else);
      append_block_stmts(state, then, if_stmt->then_stmts, end);
      if (_else != NULL) {
        append_block_stmts(state, _else, if_stmt->else_stmts, end);
      }
      LLVMPositionBuilderAtEnd(builder, end);
    });
    CASE(while_loop, {
      auto cond = LLVMAppendBasicBlockInContext(ctx, state->fn, "cond");
      auto body = LLVMAppendBasicBlockInContext(ctx, state->fn, "body");
      auto end = LLVMAppendBasicBlockInContext(ctx, state->fn, "end");
      LLVMBuildBr(builder, cond);
      LLVMPositionBuilderAtEnd(builder, cond);
      GEN(while_loop->cond, cond_value);
      LLVMBuildCondBr(builder, cond_value, body, end);
      append_block_stmts(state, body, while_loop->stmts, cond);
      LLVMPositionBuilderAtEnd(builder, end);
    });
    CASE(for_loop, {
      GEN(for_loop->init, init_value);
      auto cond = LLVMAppendBasicBlockInContext(ctx, state->fn, "cond");
      auto body = LLVMAppendBasicBlockInContext(ctx, state->fn, "body");
      auto end = LLVMAppendBasicBlockInContext(ctx, state->fn, "end");
      LLVMBuildBr(builder, cond);
      LLVMPositionBuilderAtEnd(builder, cond);
      GEN(for_loop->cond, cond_value);
      LLVMBuildCondBr(builder, cond_value, body, end);

      LLVMPositionBuilderAtEnd(builder, body);
      ITER_ARRAY(for_loop->stmts, stmt_node, GEN(stmt_node, stmt_value));
      GEN(for_loop->step, step_value);
      LLVMBuildBr(
          builder,
          cond); // TODO: skip branch if unconditional return stmt in body

      LLVMPositionBuilderAtEnd(builder, end);
    });
    CASE(assign, {
      LLVMValueRef target_ptr = NULL;
      if (assign->target->kind == aPTR_DEREF) {
        GEN(assign->target->ptr_deref.expr, _target_ptr);
        target_ptr = _target_ptr;
      } else {
        assert(assign->target->kind == aNAME);
        auto target_name = &assign->target->name;
        // either name->decl != NULL or the variable refers to a builtin,
        // which is not assignable
        assert(target_name->decl != NULL);
        if (target_name->decl->llvm_value == NULL) {
          panicf("unimplemented: forward declaration of assignment target");
        }
        target_ptr = target_name->decl->llvm_value;
      }
      GEN(assign->expr, expr_value);
      if (assign->op == tEQ) {
        LLVMBuildStore(builder, expr_value, target_ptr);
        break;
      }
      GEN(assign->target, target_value);
      switch (assign->op) {
        ASS(PLUS_EQ, Add);
        ASS(MINUS_EQ, Sub);
        ASS(STAR_EQ, Mul);
        ASS(SLASH_EQ, SDiv);
      default:
        panicf("not an assignment operator: %s\n", token_name(assign->op));
      }
    });
    CASE(return_stmt, {
      GEN(return_stmt->expr, expr_value);
      LLVMBuildRet(builder, expr_value);
    });
    CASE(field, { panicf("unreachable"); });
    CASE(struct_decl, {
      auto struct_type = declare_struct(state->ctx, node);
      LLVMTypeRef element_types[MAX(struct_decl->fields.length, 1)];
      ITER_ARRAY(struct_decl->fields, field_node,
                 { element_types[i] = TO_LLVM_TYPE(field_node->type); });
      LLVMStructSetBody(struct_type, element_types, struct_decl->fields.length,
                        false);
    });
    CASE(field_inst, { panicf("field_inst should not be analyzed directly"); });
    CASE(struct_inst, {
      auto type = TO_LLVM_TYPE(node->type);
      auto value_ptr = LLVMBuildAlloca(builder, type, "");

      ITER_ARRAY(struct_inst->fields, field_node, {
        auto field_ptr = LLVMBuildStructGEP2(builder, type, value_ptr, i, "");
        GEN(field_node->field_inst.expr, field_value);
        LLVMBuildStore(builder, field_value, field_ptr);
      });
      node->llvm_value = LLVMBuildLoad2(builder, type, value_ptr, "");
    });
    CASE(member, {
      GEN(member->expr, expr_value);
      auto name = member->name->name.name;
      auto name_string = strndup(name.text, name.length);
      node->llvm_value = LLVMBuildExtractValue(
          builder, expr_value, member->field_index, name_string);
      free(name_string);
    });
    CASE(var_decl, {
      char *name = strndup(var_decl->name->name.name.text,
                           var_decl->name->name.name.length);
      auto type = TO_LLVM_TYPE(var_decl->expr->type);
      char *type_string = LLVMPrintTypeToString(type);
      eprintf("Generating var_decl `%s`: %s\n", name, type_string);
      LLVMDisposeMessage(type_string);

      if (var_decl->is_global) {
        node->llvm_value = LLVMAddGlobal(mod, type, name);
        if (!IS_ONE_OF(node->type.kind, tyINT, tyBOOL, tySTRING)) {
          // NOTE: probably will never implement initializers that call
          // functions, but structs should be able to be used as global
          // variables
          panicf("unimplemented: complex global variable initializers");
        }
        GEN(var_decl->expr, expr_value);
        LLVMSetInitializer(node->llvm_value, expr_value);
      } else { // local
        // TODO: handle local constants differently
        // TODO: only place decl on stack when it needs an address
        node->llvm_value = LLVMBuildAlloca(builder, type, name);
        GEN(var_decl->expr, expr_value);
        LLVMBuildStore(builder, expr_value, node->llvm_value);
      }
      free(name);
    });
    CASE(module, {
      ITER_ARRAY(module->decls, decl_node, { GEN(decl_node, decl_value); });
    });
  });
}

// TODO: user-exposed init function (or keep track of initialization with
// boolean)
void init_llvm(void) {
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

#define ADD_BUILTIN_FUNCTION($name, $type)                                     \
  LLVMAddFunction(mod, $name, to_llvm_type(ctx, prim, $type))

void generate_and_evaluate(Node *node) {
  assert(node->kind == aMODULE);
  auto ctx = LLVMContextCreate();
  auto mod = LLVMModuleCreateWithNameInContext("placeholder_module_name", ctx);
  auto builder = LLVMCreateBuilderInContext(ctx);
  // TODO: LLVMIntPtrTypeInContext (also change builtin.h: print_int and
  // CeamString)
  auto _int = LLVMInt64TypeInContext(ctx);
  auto ptr = LLVMPointerTypeInContext(ctx, 0);
  auto string = LLVMStructCreateNamed(ctx, "string");
  LLVMTypeRef elementTypes[2] = {ptr, _int};
  LLVMStructSetBody(string, elementTypes, 2, false);

  eprintf("Generating module LLVM.\n");

  Primitives prim = {
      ._void = LLVMVoidTypeInContext(ctx),
      ._int = _int,
      ._bool = LLVMInt1TypeInContext(ctx),
      .ptr = ptr,
      .string = string,
  };
  State state = {
      .ctx = ctx,
      .mod = mod,
      .builder = builder,
      .builtin =
          {
              .print_bool = ADD_BUILTIN_FUNCTION("print_bool", PRINT_BOOL_TYPE),
              .print_int = ADD_BUILTIN_FUNCTION("print_int", PRINT_INT_TYPE),
              .print_string =
                  ADD_BUILTIN_FUNCTION("print_string", PRINT_STRING_TYPE),
          },
      .prim = prim};

  generate_node(&state, node);

  eprintf("Verifying module LLVM.\n");

  char *mod_string = LLVMPrintModuleToString(mod);
  eprintf("Module START\n%s", mod_string);
  LLVMDisposeMessage(mod_string);
  eprintf("Module END\n");

  LLVMVerifyModule(mod, LLVMAbortProcessAction, NULL);
  eprintf("Running module main function.\n");
  auto main_fn = LLVMGetNamedFunction(mod, "main");
  if (main_fn == NULL) {
    panicf("unimplemented: libraries without main()");
  }
  init_llvm();
  LLVMExecutionEngineRef engine = NULL;
  char *error = NULL;
  // Takes ownership of mod.
  if (LLVMCreateExecutionEngineForModule(&engine, mod, &error)) {
    panicf("Failed to create LLVM execution engine. Error: %s", error);
  }
  LLVMDisposeMessage(error);
  void (*main_ptr)(void) = LLVMGetPointerToGlobal(engine, main_fn);
  if (main_ptr == NULL) {
    panicf("Failed to get pointer to main function in module.");
  }
  main_ptr();

  LLVMDisposeExecutionEngine(engine);
  LLVMDisposeBuilder(builder);
  LLVMContextDispose(ctx);
  eprintf("Finished generation and execution.\n");
}
