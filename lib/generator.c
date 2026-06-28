
#include <llvm-c/Analysis.h>
#include <llvm-c/Core.h>
#include <llvm-c/ExecutionEngine.h>
#include <llvm-c/IRReader.h>
#include <llvm-c/Linker.h>
#include <llvm-c/Support.h>
#include <llvm-c/Target.h>
#include <llvm-c/TargetMachine.h>
#include <llvm-c/Types.h>

#include "ast.h"
#include "ast_macro.h"
#include "generator.h"
#include "llvm.h"

typedef ASTNode Node;

typedef struct {
  LLVMBasicBlockRef break_to;
  LLVMBasicBlockRef continue_to;
} LoopBlocks;

typedef struct {
  LLVMValueRef print_bool;
  LLVMValueRef print_int;
  LLVMValueRef print_str;
  LLVMValueRef print_char;
  LLVMValueRef print_byte;
  LLVMValueRef __str_eq;
} LLVMBuiltins;

typedef struct State {
  LLVMContextRef ctx;
  LLVMModuleRef mod;
  LLVMBuilderRef builder;
  LLVMValueRef fn;
  LoopBlocks loop;
  LLVMBuiltins builtin;
  LLVMPrimitives prim;
} State;

#define GEN($node, $name)                                                      \
  generate_node(state, $node);                                                 \
  LLVMValueRef $name = $node->llvm_value;                                      \
  UNUSED($name)

#define BUILD($instr, ...) LLVMBuild##$instr(builder __VA_OPT__(, ) __VA_ARGS__)

#define BIN($op, $instr)                                                       \
  case t##$op:                                                                 \
    node->llvm_value = BUILD($instr, left_value, right_value, "");             \
    break;

#define CMP($op, $pred)                                                        \
  case t##$op:                                                                 \
    node->llvm_value = BUILD(                                                  \
        ICmp, is_signed(node->type.kind) ? LLVMIntS##$pred : LLVMIntU##$pred,  \
        left_value, right_value, "");                                          \
    break;

#define TO_LLVM_TYPE($type) to_llvm_type(state->ctx, state->prim, $type)

LLVMValueRef declare_function(State *state, ASTNode *decl_node) {
  assert(decl_node->kind == aFUNC_DECL);
  if (decl_node->llvm_value != NULL) {
    return decl_node->llvm_value;
  }
  auto decl = &decl_node->func_decl;
  auto decl_name = decl->name->name.name;
  // map main -> _main because a wrapper main function is automatically
  // generated
  char *name = name_eq_string(decl_name, "main")
                   ? strdup("_main")
                   : strndup(decl_name.text, decl_name.length);
  auto type = TO_LLVM_TYPE(decl_node->type);

  char *type_string = LLVMPrintTypeToString(type);
  eprintf("Generating function decl %s: %s\n", name, type_string);
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
  bool terminated = false;
  ITER_ARRAY(stmts, stmt_node, {
    GEN(stmt_node, stmt_value);
    if (LLVMGetBasicBlockTerminator(block) != NULL) {
      terminated = true;
      break; // skip statements after terminator in this block
    }
  });
  if (!terminated) {
    BUILD(Br, next_block);
  }
}

#define ASS($op, $instr)                                                       \
  case t##$op: {                                                               \
    auto new_value = BUILD($instr, target_value, expr_value, "");              \
    BUILD(Store, new_value, target_ptr);                                       \
    break;                                                                     \
  }

#define PRINT_TYPE_STRING($type, $format, ...)                                 \
  {                                                                            \
    char *type_string = LLVMPrintTypeToString($type);                          \
    eprintf($format, __VA_ARGS__);                                             \
    LLVMDisposeMessage(type_string);                                           \
  }

LLVMValueRef generate_pointer(State *state, Node *node) {
  auto builder = state->builder;
  if (node->kind == aPTR_DEREF) {
    GEN(node->ptr_deref.expr, ptr);
    return ptr;
  } else if (node->kind == aINDEX_OP) {
    auto index_op = &node->index_op;
    GEN(index_op->expr, expr_value);
    GEN(index_op->index, index_value);
    Type expr_type = index_op->expr->type;
    LLVMValueRef array_ptr = NULL;
    if (expr_type.kind == tySTRING) {
      array_ptr = BUILD(ExtractValue, expr_value, 0, "");
    } else {
      assert(expr_type.kind == tyARRAY);
      assert(index_op->expr->type.kind == tyPTR);
      array_ptr = expr_value;
    }
    // TODO: bounds check (LLVMBuildInBoundsGEP2)
    auto elem_type = TO_LLVM_TYPE(node->type);
    return BUILD(GEP2, elem_type, array_ptr, &index_value, 1, "");
  } else {
    assert(node->kind == aNAME);
    auto name = &node->name;
    // assignable targets are always declarations
    assert(name->decl != NULL);
    if (name->decl->llvm_value == NULL) {
      panicf("unimplemented: forward declaration of assignment target");
    }
    return name->decl->llvm_value;
  }
}

void generate_node(State *state, Node *node) {
  auto ctx = state->ctx;
  auto mod = state->mod;
  auto builder = state->builder;
  SWITCH(node, {
    CASE(name, {
      if (name->decl == NULL) {
        panicf(
            "unimplemented: builtins and external declarations as variables");
      }
      if (name->decl->llvm_value == NULL) {
        panicf("unimplemented: forward declarations");
      }
      char *label = strndup(name->name.text, name->name.length);
      node->llvm_value =
          BUILD(Load2, TO_LLVM_TYPE(node->type), name->decl->llvm_value, label);
      free(label);
    });
    CASE(integer, node->llvm_value =
                      LLVMConstInt(state->prim._int, integer->value, true));
    CASE(boolean, node->llvm_value =
                      LLVMConstInt(state->prim._bool, boolean->value, false));
    // creates an immutable string slice
    CASE(string, {
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
      node->llvm_value = LLVMConstNamedStruct(state->prim.str, value, 2);
    });
    CASE(character, {
      node->llvm_value = LLVMConstInt(state->prim.i32, character->value, false);
    });
    CASE(unary, {
      GEN(unary->expr, expr_value);
      switch (unary->op) {
      case tBANG:
        node->llvm_value = BUILD(Not, expr_value, "");
        break;
      case tMINUS:
        node->llvm_value = BUILD(Neg, expr_value, "");
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
        CMP(LT, LT);
        CMP(GT, GT);
        CMP(LT_EQ, LE);
        CMP(GT_EQ, GE);
      case tEQ_EQ:
        if (IS_ONE_OF(binary->left->type.kind, tyINT, tyBOOL)) {
          node->llvm_value =
              BUILD(ICmp, LLVMIntEQ, left_value, right_value, "");
        } else {
          assert(binary->left->type.kind == tySTR);
          LLVMValueRef args[2] = {left_value, right_value};
          node->llvm_value = BUILD(Call2, TO_LLVM_TYPE(STR_EQ_TYPE),
                                   state->builtin.__str_eq, args, 2, "");
        }
        break;
      case tBANG_EQ: {
        auto eq_value = BUILD(ICmp, LLVMIntEQ, left_value, right_value, "");
        node->llvm_value = BUILD(Not, eq_value, "");
        break;
      }
        // TODO: short-circuiting AND/OR
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
      } else if (function->name.builtin != NOT_BUILTIN) {
        switch (function->name.builtin) {
        case bPRINT_BOOL:
          fn = state->builtin.print_bool;
          break;
        case bPRINT_INT:
          fn = state->builtin.print_int;
          break;
        case bPRINT_STR:
          fn = state->builtin.print_str;
          break;
        case bPRINT_CHAR:
          fn = state->builtin.print_char;
          break;
        case bPRINT_BYTE:
          fn = state->builtin.print_byte;
          break;
        default:
          panicf("Unknown builtin %d.", function->name.builtin);
        }
      } else {
        assert(function->name.extern_decl != NULL);
        assert(function->name.extern_decl->llvm_value != NULL);
        fn = function->name.extern_decl->llvm_value;
      }
      size_t num_args = func_call->args.length;
      LLVMValueRef args[MAX(num_args, 1)];
      ITER_ARRAY(func_call->args, arg_node, {
        GEN(arg_node, arg_value);
        args[i] = arg_value;
      });
      node->llvm_value =
          BUILD(Call2, TO_LLVM_TYPE(function->type), fn, args, num_args, "");
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
      node->llvm_value = BUILD(Load2, TO_LLVM_TYPE(node->type), ptr_value, "");
    });
    CASE(ptr_type, { panicf("unreachable"); });
    CASE(index_op, {
      auto elem_type = TO_LLVM_TYPE(node->type);
      node->llvm_value =
          BUILD(Load2, elem_type, generate_pointer(state, node), "");
    });
    // Functions are generated through decl so that they can be declared with a
    // name in LLVM.
    CASE(func_decl, {
      auto fn = declare_function(state, node);
      node->llvm_value = fn;
      if (!func_decl->is_forward_decl) {
        state->fn = fn;
        auto entry_block = LLVMAppendBasicBlockInContext(ctx, fn, "entry");
        LLVMPositionBuilderAtEnd(builder, entry_block);

        ITER_ARRAY(func_decl->params, param_node, {
          // TODO: only place parameter on stack when it needs an address
          param_node->llvm_value =
              BUILD(Alloca, TO_LLVM_TYPE(param_node->type), "");
          BUILD(Store, LLVMGetParam(fn, i), param_node->llvm_value);
        });
        ITER_ARRAY(func_decl->stmts, stmt_node,
                   { GEN(stmt_node, stmt_value); });
        if (func_decl->return_type == NULL) {
          BUILD(RetVoid);
        }
        state->fn = NULL;
      }
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
      BUILD(CondBr, cond_value, then, _else == NULL ? end : _else);
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
      state->loop = (LoopBlocks){.break_to = end, .continue_to = cond};
      // cond
      BUILD(Br, cond);
      LLVMPositionBuilderAtEnd(builder, cond);
      GEN(while_loop->cond, cond_value);
      // body
      BUILD(CondBr, cond_value, body, end);
      append_block_stmts(state, body, while_loop->stmts, cond);
      // end
      LLVMPositionBuilderAtEnd(builder, end);
      state->loop = (LoopBlocks){0};
    });
    CASE(for_loop, {
      auto cond = LLVMAppendBasicBlockInContext(ctx, state->fn, "cond");
      auto body = LLVMAppendBasicBlockInContext(ctx, state->fn, "body");
      auto step = LLVMAppendBasicBlockInContext(ctx, state->fn, "step");
      auto end = LLVMAppendBasicBlockInContext(ctx, state->fn, "end");
      state->loop = (LoopBlocks){.break_to = end, .continue_to = step};
      // init
      GEN(for_loop->init, init_value);
      // cond
      BUILD(Br, cond);
      LLVMPositionBuilderAtEnd(builder, cond);
      GEN(for_loop->cond, cond_value);
      BUILD(CondBr, cond_value, body, end);
      // body
      append_block_stmts(state, body, for_loop->stmts, step);
      // step
      LLVMPositionBuilderAtEnd(builder, step);
      GEN(for_loop->step, step_value);
      BUILD(Br, cond);
      // end
      LLVMPositionBuilderAtEnd(builder, end);
      state->loop = (LoopBlocks){0};
    });
    CASE(assign, {
      auto target_ptr = generate_pointer(state, assign->target);
      GEN(assign->expr, expr_value);
      if (assign->op == tEQ) {
        BUILD(Store, expr_value, target_ptr);
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
      BUILD(Ret, expr_value);
    });
    CASE(break_stmt, {
      assert(state->loop.continue_to != NULL);
      BUILD(Br, state->loop.break_to);
    });
    CASE(continue_stmt, {
      assert(state->loop.break_to != NULL);
      BUILD(Br, state->loop.continue_to);
    });
    CASE(field, { panicf("field should not be analyzed directly"); });
    CASE(struct_decl, {
      auto struct_type = llvm_declare_struct(state->ctx, node);

      LLVMTypeRef element_types[MAX(struct_decl->fields.length, 1)];
      ITER_ARRAY(struct_decl->fields, field_node,
                 { element_types[i] = TO_LLVM_TYPE(field_node->type); });
      LLVMStructSetBody(struct_type, element_types, struct_decl->fields.length,
                        false);

      char *struct_name = name_dup_to_string(struct_decl->name->name.name);
      PRINT_TYPE_STRING(struct_type, "Generating struct_decl %s\n",
                        type_string);
      free(struct_name);
    });
    CASE(field_inst, { panicf("field_inst should not be analyzed directly"); });
    CASE(struct_inst, {
      auto type = TO_LLVM_TYPE(node->type);
      auto value_ptr = BUILD(Alloca, type, "");

      ITER_ARRAY(struct_inst->fields, field_node, {
        auto field_ptr = BUILD(StructGEP2, type, value_ptr,
                               field_node->field_inst.index, "");
        GEN(field_node->field_inst.expr, field_value);
        BUILD(Store, field_value, field_ptr);
      });
      node->llvm_value = BUILD(Load2, type, value_ptr, "");
    });
    CASE(member, {
      GEN(member->expr, struct_value);
      auto name = member->name->name.name;
      auto name_string = strndup(name.text, name.length);
      node->llvm_value =
          BUILD(ExtractValue, struct_value, member->field_index, name_string);
      free(name_string);
    });
    CASE(var_decl, {
      char *name = strndup(var_decl->name->name.name.text,
                           var_decl->name->name.name.length);
      auto type = TO_LLVM_TYPE(var_decl->expr->type);

      PRINT_TYPE_STRING(type, "Generating var_decl %s: %s\n", name,
                        type_string);

      if (var_decl->is_global) {
        node->llvm_value = LLVMAddGlobal(mod, type, name);
        GEN(var_decl->expr, expr_value);
        LLVMSetInitializer(node->llvm_value, expr_value);
      } else { // local
        // TODO: handle local constants differently
        // TODO: only place decl on stack when it needs an address
        node->llvm_value = BUILD(Alloca, type, name);
        GEN(var_decl->expr, expr_value);
        BUILD(Store, expr_value, node->llvm_value);
      }
      free(name);
    });
    CASE(module, {
      ITER_ARRAY(module->decls, decl_node, { GEN(decl_node, decl_value); });
    });
  });
}

#define ADD_BUILTIN_FUNCTION($name, $type)                                     \
  builtin.$name = LLVMAddFunction(mod, #$name, to_llvm_type(ctx, prim, $type));

LLVMBuiltins add_builtins(LLVMContextRef ctx, LLVMModuleRef mod,
                          LLVMPrimitives prim) {
  LLVMBuiltins builtin = {0};
  ADD_BUILTIN_FUNCTION(print_bool, PRINT_BOOL_TYPE);
  ADD_BUILTIN_FUNCTION(print_int, PRINT_INT_TYPE);
  ADD_BUILTIN_FUNCTION(print_str, PRINT_STR_TYPE);
  ADD_BUILTIN_FUNCTION(print_char, PRINT_CHAR_TYPE);
  ADD_BUILTIN_FUNCTION(print_byte, PRINT_BYTE_TYPE);
  ADD_BUILTIN_FUNCTION(__str_eq, STR_EQ_TYPE);
  return builtin;
}

// TODO: only declare extern declarations if they are used
void add_extern_decls(ExternMod *extern_mod, State *state) {
  for (size_t i = 0; i < extern_mod->decls.length; i++) {
    auto decl = &extern_mod->decls.data[i];
    if (decl->type.kind == tyFUNCTION) {
      char *name = strndup(decl->name.text, decl->name.length);
      auto type = TO_LLVM_TYPE(decl->type);

      char *type_string = LLVMPrintTypeToString(type);
      eprintf("Generating function decl `%s`: %s\n", name, type_string);
      LLVMDisposeMessage(type_string);

      decl->llvm_value = LLVMAddFunction(state->mod, name, type);
      free(name);
    } else {
      panicf("unimplemented: extern variable declaration\n");
    }
  }
}

LLVMModuleRef generate_llvm(LLVMContextRef ctx, AST *ast) {
  auto node = ast->head;
  auto extern_mod = &ast->extern_mod;
  assert(node->kind == aMODULE);
  auto mod = LLVMModuleCreateWithNameInContext("placeholder_module_name", ctx);
  auto builder = LLVMCreateBuilderInContext(ctx);
  // TODO: LLVMIntPtrTypeInContext (also change builtin.h: print_int and
  // CeamString)
  auto _int = LLVMInt64TypeInContext(ctx);
  auto ptr = LLVMPointerTypeInContext(ctx, 0);
  auto string = LLVMStructCreateNamed(ctx, "string");
  LLVMTypeRef elementTypes[2] = {ptr, _int};
  LLVMStructSetBody(string, elementTypes, 2, false);

  eprintf("Generating LLVM module.\n");

  // TODO: move LLVMPrimitives to LLVMState?
  LLVMPrimitives prim = {
      ._void = LLVMVoidTypeInContext(ctx),
      ._bool = LLVMInt1TypeInContext(ctx),
      .i8 = LLVMInt8TypeInContext(ctx),
      .i16 = LLVMInt16TypeInContext(ctx),
      .i32 = LLVMInt32TypeInContext(ctx),
      .i64 = LLVMInt64TypeInContext(ctx),
      ._int = _int,
      .ptr = ptr,
      .str = string,
  };
  State state = {.ctx = ctx,
                 .mod = mod,
                 .builder = builder,
                 .builtin = add_builtins(ctx, mod, prim),
                 .prim = prim};

  add_extern_decls(extern_mod, &state);
  generate_node(&state, node);

  LLVMDumpModule(mod); // TEMP

  eprintf("Verifying LLVM module.\n");
  LLVMVerifyModule(mod, LLVMAbortProcessAction, NULL);

  LLVMDisposeBuilder(builder);
  eprintf("Finished LLVM module generation.\n");

  return mod;
}
