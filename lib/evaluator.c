
#include "evaluator.h"
#include "analyzer_shared.h"
#include "ast.h"
#include "ast_macro.h"

#include <stdalign.h> // TODO: remove dependency

#ifdef DEBUG_EVALUATOR
#include "offset.h"

const char *evaluator_source;

#define LOG_ENTER                                                              \
  {                                                                            \
    OffsetInfo oi = get_offset_info(evaluator_source, node->span.offset);      \
    eprintf("%-3zu %-2zu | %.*s | %s\n", oi.line_number, recursion_depth,      \
            (int)oi.line_length, oi.line, ast_node_name(node->kind));          \
  }
#else
#define LOG_ENTER
#endif

typedef ASTNodeArray NodeArray;

typedef enum {
  cNEXT = 0,
  cRETURN,
} ControlFlow;

#define RETURN(control_flow)                                                   \
  {                                                                            \
    return control_flow;                                                       \
  }

// TODO: alignment dependent on value(s) stored
#define FRAME($name, $size)                                                    \
  /* C requires that a variable-length array contains at least 1 element.*/    \
  char $name##__buf[MAX($size + (alignof(Value) - 1), 1)];                     \
  char *$name = $name##__buf;                                                  \
  {                                                                            \
    size_t __offset = alignof(Value) - (uintptr_t)$name % alignof(Value);      \
    if (__offset != alignof(Value))                                            \
      $name = $name + __offset;                                                \
  }                                                                            \
  memset($name##__buf, 0, sizeof($name##__buf))

#define SLOT($name, $type) FRAME($name, size_of($type))

#define EVALUATE($node, $name)                                                 \
  SLOT($name, $node->type);                                                    \
  evaluate_expr($node, recursion_depth, stack_frame, $name);

#define EVALUATE_ARRAY($array)                                                 \
  ITER_ARRAY($array, element, { EVALUATE(element, element); });

#define PARAMS                                                                 \
  Node *node, size_t recursion_depth, char *stack_frame, char *function_out

#define EVALUATE_STMT($stmt)                                                   \
  evaluate_stmt($stmt, recursion_depth, stack_frame, function_out)
#define EVALUATE_STMTS($stmts)                                                 \
  evaluate_stmts($stmts, recursion_depth, stack_frame, function_out)

ControlFlow evaluate_stmt(PARAMS);

ControlFlow evaluate_stmts(NodeArray stmts, size_t recursion_depth,
                           char *stack_frame, char *function_out) {
  ITER_ARRAY(stmts, stmt_node, {
    ControlFlow control = EVALUATE_STMT(stmt_node);
    if (control == cRETURN)
      return cRETURN;
  });
  return cNEXT;
}

char *evaluate_target(Node *node, char *stack_frame) {
  SWITCH(node, {
    CASE(name, {
      switch (name->value.kind) {
      case symBUILTIN:
        panicf("cannot take address of builtin");
      case symDYNAMIC:
        return &stack_frame[name->value.local_index];
      case symSTATIC:
        return name->value.static_ptr;
      }
      panicf("unreachable");
    });
  default:
    panicf("not a target: %s", ast_node_name(node->kind));
  });
}

#define EVALUATOR($name, $body...)                                             \
  ControlFlow evaluate_##$name(PARAMS) {                                       \
    LOG_ENTER;                                                                 \
    __auto_type $name = &node->$name;                                          \
    UNUSED(recursion_depth);                                                   \
    UNUSED(stack_frame);                                                       \
    UNUSED(function_out);                                                      \
    $body;                                                                     \
  }

EVALUATOR(if_stmt, {
  EVALUATE(if_stmt->cond, cond_value);
  ControlFlow control = 0;
  if (AS(cond_value, bool)) {
    control = EVALUATE_STMTS(if_stmt->then_stmts);
  } else {
    control = EVALUATE_STMTS(if_stmt->else_stmts);
  }
  RETURN(control);
});

EVALUATOR(while_loop, {
  while (true) {
    EVALUATE(while_loop->cond, cond_value);
    if (!AS(cond_value, bool))
      break;
    ControlFlow control = EVALUATE_STMTS(while_loop->stmts);
    if (control == cRETURN)
      RETURN(cRETURN);
  }
  RETURN(cNEXT);
});

EVALUATOR(for_loop, {
  EVALUATE_STMT(for_loop->init);
  while (true) {
    EVALUATE(for_loop->cond, cond_value);
    if (!AS(cond_value, bool))
      break;
    ControlFlow control = EVALUATE_STMTS(for_loop->stmts);
    if (control == cRETURN)
      RETURN(cRETURN);
    EVALUATE_STMT(for_loop->step);
  }
  RETURN(cNEXT);
});

#define BIN_ASSIGN($op) AS(target, ssize_t) $op AS(expr_value, ssize_t)

EVALUATOR(assign, {
  char *target = evaluate_target(assign->target, stack_frame);
  EVALUATE(assign->expr, expr_value);
  switch (assign->op) {
  case tEQ:
    AS(target, bool) = AS(expr_value, bool);
    break;
  case tPLUS_EQ:
    BIN_ASSIGN(+=);
    break;
  case tMINUS_EQ:
    BIN_ASSIGN(-=);
    break;
  case tSTAR_EQ:
    BIN_ASSIGN(*=);
    break;
  case tSLASH_EQ:
    BIN_ASSIGN(/=);
    break;
  default:
    panicf("Unexpected assignment operator: %s", token_name(assign->op));
  }
  RETURN(cNEXT);
});

EVALUATOR(return_stmt, {
  EVALUATE(return_stmt->expr, expr_value);
  eprintf("returning %zd\n", AS(expr_value, ssize_t));
  *function_out = *expr_value;
  RETURN(cRETURN);
});

EVALUATOR(decl, {
  // FIXME: should be is_static
  if (!decl->is_constant) {
    EVALUATE(decl->expr, value);
    memcpy(&stack_frame[decl->local_index], value, size_of(node->type));
  }
  RETURN(cNEXT);
});

#define ECASE($name)                                                           \
  CASE($name, return evaluate_##$name(node, recursion_depth, stack_frame,      \
                                      function_out))

ControlFlow evaluate_stmt(PARAMS) {
  SWITCH(node, {
    ECASE(if_stmt);
    ECASE(while_loop);
    ECASE(for_loop);
    ECASE(assign);
    ECASE(return_stmt);
    ECASE(decl);
  default: {
    SLOT(value, node->type);
    evaluate_expr(node, recursion_depth, stack_frame, value);
    RETURN(cNEXT);
  }
  });
}
#undef PARAMS
#undef RETURN
#undef EVALUATOR
#undef ECASE

void evaluate_builtin(NodeArray args, BuiltinID id, size_t recursion_depth,
                      char *stack_frame) {
  switch (id) {
  case NOT_BUILTIN:
    panicf("unreachable");
  case PRINT_STRING: {
    assert(args.length == 1);
    EVALUATE(args.data[0], arg_value);
    String arg_string = AS(arg_value, String);
    // Using fwrite instead of printf because printf can only print
    // non-zero-delimited strings of up to INT_MAX characters in length.
    fwrite(arg_string.text, arg_string.length, 1, stdout);
    return;
  }
  default:
    panicf("unimplemented builtin: %d", id);
  }
}

#define EVALUATOR($name, ...)                                                  \
  void evaluate_##$name(Node *node, size_t recursion_depth, char *stack_frame, \
                        char *out) {                                           \
    LOG_ENTER;                                                                 \
    __auto_type $name = &node->$name;                                          \
    UNUSED($name);                                                             \
    UNUSED(recursion_depth);                                                   \
    UNUSED(stack_frame);                                                       \
    __VA_ARGS__;                                                               \
  }

#define OK return

#define RETURN($value...)                                                      \
  {                                                                            \
    memcpy(out, $value, size_of(node->type));                                  \
    OK;                                                                        \
  }

#define RETURN_ONE($value...)                                                  \
  {                                                                            \
    AS(out, typeof($value)) = $value;                                          \
    OK;                                                                        \
  }

const char *symbol_value_name(int kind) {
  switch (kind) {
  case symBUILTIN:
    return "BUILTIN";
  case symSTATIC:
    return "STATIC";
  case symDYNAMIC:
    return "DYNAMIC";
  default:
    return "<unknown kind>";
  }
}

EVALUATOR(name, {
  eprintf("evaluating name %.*s (%s %s)\n", FMT(name->name),
          symbol_value_name(name->value.kind), type_name(node->type.kind));
  switch (name->value.kind) {
  case symBUILTIN: {
    eprintf("builtin `%.*s`: %zd\n", FMT(name->name), name->value.builtin._int);
    RETURN(&name->value.builtin);
  }
  case symSTATIC: {
    if (name->value.static_ptr == NULL)
      panicf("static_ptr == NULL");
    eprintf("static `%.*s` at %p\n", FMT(name->name), name->value.static_ptr);
    RETURN(name->value.static_ptr);
  }
  case symDYNAMIC: {
    size_t local_index = name->value.local_index;
    eprintf("dynamic %zu `%.*s` at %p\n", local_index, FMT(name->name),
            &stack_frame[local_index]);
    RETURN(&stack_frame[local_index]);
  }
  }
  panicf("unreachable");
});

EVALUATOR(integer, { RETURN_ONE(integer->value); });
EVALUATOR(boolean, { RETURN_ONE(boolean->value); });
EVALUATOR(string, { RETURN_ONE(string->value); });

EVALUATOR(unary, {
  EVALUATE(unary->expr, expr_value);
  if (unary->op == tMINUS) {
    RETURN_ONE(-AS(expr_value, ssize_t));
  }
  panicf("Unknown unary operator: `%s`", token_display_name(unary->op));
});

#define BIN($out_type, $in_type, $op)                                          \
  AS(&value, $out_type) =                                                      \
      AS(left_value, $in_type) $op AS(right_value, $in_type);                  \
  break;

#define ARITH_BIN($op) BIN(ssize_t, ssize_t, $op)
#define CMP_BIN($op) BIN(bool, ssize_t, $op)
#define BOOL_BIN($op) BIN(bool, ssize_t, $op)

EVALUATOR(binary, {
  EVALUATE(binary->left, left_value);
  EVALUATE(binary->right, right_value);
  Value value = {0};
  switch (binary->op) {
  case tPLUS:
    ARITH_BIN(+);
  case tMINUS:
    ARITH_BIN(-);
  case tSTAR:
    ARITH_BIN(*);
  case tSLASH:
    ARITH_BIN(/);
  case tLT:
    CMP_BIN(<);
  case tGT:
    CMP_BIN(>);
  case tLT_EQ:
    CMP_BIN(<=);
  case tGT_EQ:
    CMP_BIN(>=);
  case tEQ_EQ:
    CMP_BIN(==);
  case tAMP_AMP:
    BOOL_BIN(&&);
  case tBAR_BAR:
    BOOL_BIN(||);
  default:
    panicf("Unknown binary operator: `%s`", token_display_name(binary->op));
  }
  eprintf("%zd %s %zd -> %zd\n", AS(left_value, ssize_t),
          token_display_name(binary->op), AS(right_value, ssize_t), value._int);
  RETURN(&value);
});

EVALUATOR(function_call, {
  EVALUATE(function_call->function, value);
  FunctionValue function_value = AS(value, FunctionValue);

  if (function_value.builtin != NOT_BUILTIN) {
    evaluate_builtin(function_call->args, function_value.builtin,
                     recursion_depth + 1, stack_frame);
    OK;
  }
  assert(function_value.ptr->kind == aFUNCTION);
  __auto_type function = &function_value.ptr->function;
  FRAME(new_stack_frame, function->frame_length);
  eprintf("function call (%zu locals, %zu params, stack frame at %p)\n",
          function->frame_length, function->params.length, new_stack_frame);

  size_t arg_local_index = 0;
  ITER_ARRAY(function_call->args, arg_node, {
    eprintf("evaluating arg %zu\n", i);
    char *arg_value = &new_stack_frame[arg_local_index];
    evaluate_expr(arg_node, recursion_depth + 1, stack_frame, arg_value);
    arg_local_index += size_of(arg_node->type);
    eprintf("set param %zu at %p to %zd\n", i, arg_value,
            AS(arg_value, ssize_t));
  });
  // Assumes that parameter values have been set.
  SLOT(function_out, node->type);
  evaluate_stmts(function->stmts, recursion_depth + 1, new_stack_frame,
                 function_out);
  RETURN(function_out);
});

EVALUATOR(function, {
  UNUSED(function);
  RETURN_ONE((FunctionValue){.ptr = node});
});

EVALUATOR(_struct, {
  UNUSED(_struct);
  RETURN_ONE((Type){.kind = tySTRUCT, ._struct = node});
});

EVALUATOR(struct_inst, {
  size_t out_index = 0;
  ITER_ARRAY(struct_inst->fields, field_inst_node, {
    evaluate_expr(field_inst_node->field_inst.expr, recursion_depth,
                  stack_frame, &out[out_index]);
    out_index += size_of(field_inst_node->type);
  });
  OK;
});

EVALUATOR(member, {
  SLOT(struct_value, member->expr->type);
  evaluate_expr(member->expr, recursion_depth, stack_frame, struct_value);
  memcpy(out, &struct_value[member->field_offset], member->field_size);
  OK;
});

#define ECASE($name)                                                           \
  CASE($name, return evaluate_##$name(node, recursion_depth, stack_frame, out))

void evaluate_expr(ASTNode *node, size_t recursion_depth, char *stack_frame,
                   char *out) {
  SWITCH(node, {
    ECASE(name);
    ECASE(integer);
    ECASE(boolean);
    ECASE(string);
    ECASE(unary);
    ECASE(binary);
    ECASE(function_call);
    ECASE(function);
    ECASE(_struct);
    ECASE(struct_inst);
    ECASE(member);
  default:
    panicf("unknown expression: %s", ast_node_name(node->kind));
  });
}

void evaluate_module(Node *node) {
  assert(node->kind == aMODULE);
  __auto_type module = &node->module;
  ITER_ARRAY(module->decls, decl_node, {
    __auto_type decl = &decl_node->decl;
    if (name_eq_string(decl->name->name.name, "main")) {
      evaluate_stmt(decl_node, 0, NULL, NULL);

      assert(decl->expr->kind == aFUNCTION);
      __auto_type function = &decl->expr->function;

      FRAME(stack_frame, function->frame_length);
      evaluate_stmts(function->stmts, 0, stack_frame, NULL);
      return;
    }
  });
  panicf("`main` function not found.\n");
}
