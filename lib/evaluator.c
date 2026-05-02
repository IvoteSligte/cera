
#include "evaluator.h"
#include "analyzer_shared.h"
#include "ast_macro.h"

#ifdef TEST
extern void print_string(const char *text, size_t length);
#endif

#ifdef DEBUG_EVALUATOR
#include "offset.h"
#define LOG_ENTER                                                              \
  {                                                                            \
    OffsetInfo oi = get_offset_info(node->source, node->span.offset);          \
    printf("%-3zu %-2zu | %.*s | %s\n", oi.line_number, recursion_depth,       \
           (int)oi.line_length, oi.line, ast_node_name(node->kind));           \
  }
#else
#define LOG_ENTER
#endif

typedef ASTNodeArray NodeArray;

typedef enum {
  cNEXT = 0,
  cRETURN,
} ControlFlow;

#define OK(control_flow)                                                       \
  {                                                                            \
    return control_flow;                                                       \
  }

#define EVALUATE(node, name)                                                   \
  Value name##_value = evaluate_expr(node, recursion_depth, stack_frame);      \
  UNUSED(name##_value);

#define EVALUATE_ARRAY($array)                                                 \
  ITER_ARRAY($array, element, { EVALUATE(element, element); });

#define PARAMS                                                                 \
  Node *node, size_t recursion_depth, Value *stack_frame, Value *function_out

#define EVALUATE_STMT($stmt)                                                   \
  evaluate_stmt($stmt, recursion_depth, stack_frame, function_out)
#define EVALUATE_STMTS($stmts)                                                 \
  evaluate_stmts($stmts, recursion_depth, stack_frame, function_out)

ControlFlow evaluate_stmt(PARAMS);

ControlFlow evaluate_stmts(NodeArray stmts, size_t recursion_depth,
                           Value *stack_frame, Value *function_out) {
  ITER_ARRAY(stmts, stmt_node, {
    ControlFlow control = EVALUATE_STMT(stmt_node);
    if (control == cRETURN)
      return cRETURN;
  });
  return cNEXT;
}

Value *evaluate_target(Node *node, Value *stack_frame) {
  SWITCH(node, {
    CASE(name, {
      if (name->is_static) {
        return name->static_value_ptr;
      } else {
        return &stack_frame[name->local_index];
      }
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
  EVALUATE(if_stmt->cond, cond);
  ControlFlow control = 0;
  if (cond_value.boolean) {
    control = EVALUATE_STMTS(if_stmt->then_stmts);
  } else {
    control = EVALUATE_STMTS(if_stmt->else_stmts);
  }
  OK(control);
});

EVALUATOR(while_loop, {
  while (true) {
    EVALUATE(while_loop->cond, cond);
    if (!cond_value.boolean)
      break;
    ControlFlow control = EVALUATE_STMTS(while_loop->stmts);
    if (control == cRETURN)
      OK(cRETURN);
  }
  OK(cNEXT);
});

EVALUATOR(for_loop, {
  EVALUATE_STMT(for_loop->init);
  while (true) {
    EVALUATE(for_loop->cond, cond);
    if (!cond_value.boolean)
      break;
    ControlFlow control = EVALUATE_STMTS(for_loop->stmts);
    if (control == cRETURN)
      OK(cRETURN);
    EVALUATE_STMT(for_loop->step);
  }
  OK(cNEXT);
});

EVALUATOR(assign, {
  Value *target = evaluate_target(assign->target, stack_frame);
  EVALUATE(assign->expr, expr);
  switch (assign->op) {
  case tEQ:
    *target = expr_value;
    break;
  case tPLUS_EQ:
    target->integer += expr_value.integer;
    break;
  case tMINUS_EQ:
    target->integer -= expr_value.integer;
    break;
  case tSTAR_EQ:
    target->integer *= expr_value.integer;
    break;
  case tSLASH_EQ:
    target->integer /= expr_value.integer;
    break;
  default:
    panicf("Unexpected assignment operator: %s", token_name(assign->op));
  }
  OK(cNEXT);
});

EVALUATOR(return_stmt, {
  EVALUATE(return_stmt->expr, expr);
  printf("returning %zd\n", expr_value.integer);
  *function_out = expr_value;
  OK(cRETURN);
});

EVALUATOR(decl, {
  if (!decl->is_constant) {
    EVALUATE(decl->expr, value);
    stack_frame[decl->local_index] = value_value;
  }
  OK(cNEXT);
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
  default:
    evaluate_expr(node, recursion_depth, stack_frame);
    OK(cNEXT);
  });
}
#undef PARAMS
#undef OK
#undef EVALUATOR
#undef ECASE

Value evaluate_builtin(NodeArray args, BuiltinID id, size_t recursion_depth,
                       Value *stack_frame) {
  switch (id) {
  case NOT_BUILTIN:
    panicf("unreachable");
  case PRINT_STRING: {
    assert(args.length == 1);
    EVALUATE(args.data[0], arg);
#ifdef TEST
    print_string(arg_value.string.text, arg_value.string.length);
#else
    // Using fwrite instead of printf because printf can only print
    // non-zero-delimited strings of up to INT_MAX characters in length.
    fwrite(arg_value.string.text, arg_value.string.length, 1, stdout);
#endif
    return (Value){0};
  }
  default:
    panicf("unimplemented builtin: %d", id);
  }
}

#define EVALUATOR($name, ...)                                                  \
  Value evaluate_##$name(Node *node, size_t recursion_depth,                   \
                         Value *stack_frame) {                                 \
    LOG_ENTER;                                                                 \
    __auto_type $name = &node->$name;                                          \
    UNUSED($name);                                                             \
    UNUSED(recursion_depth);                                                   \
    UNUSED(stack_frame);                                                       \
    __VA_ARGS__;                                                               \
  }

#define OK(value...) return value;

EVALUATOR(name, {
  if (name->is_static) {
    printf("static `%.*s`: %zd\n", FMT(name->name),
           name->static_value_ptr->integer);
    OK(*name->static_value_ptr);
  } else {
    printf("local %zu `%.*s` at %p: %zd\n", name->local_index, FMT(name->name),
           &stack_frame[name->local_index],
           stack_frame[name->local_index].integer);
    OK(stack_frame[name->local_index]);
  }
});

EVALUATOR(integer, { OK((Value){.integer = integer->value}); });

EVALUATOR(boolean, { OK((Value){.boolean = boolean->value}); });

EVALUATOR(string, {
  // the string value is immutable, only typed as mutable as it
  // is normally owned
  OK((Value){.string = string->value});
});

EVALUATOR(unary, {
  EVALUATE(unary->expr, expr);
  if (unary->op == tMINUS) {
    OK((Value){.integer = -expr_value.integer})
  }
  panicf("Unknown unary operator: `%s`", token_display_name(unary->op));
});

#define BIN($out_member, $member, $op)                                         \
  value.$out_member = left_value.$member $op right_value.$member;              \
  break;

EVALUATOR(binary, {
  EVALUATE(binary->left, left);
  EVALUATE(binary->right, right);
  Value value = {0};
  switch (binary->op) {
  case tPLUS:
    BIN(integer, integer, +);
  case tMINUS:
    BIN(integer, integer, -);
  case tSTAR:
    BIN(integer, integer, *);
  case tSLASH:
    BIN(integer, integer, /);
  case tLT:
    BIN(boolean, integer, <);
  case tGT:
    BIN(boolean, integer, >);
  case tLT_EQ:
    BIN(boolean, integer, <=);
  case tGT_EQ:
    BIN(boolean, integer, >=);
  case tEQ_EQ:
    BIN(boolean, integer, ==);
  case tAMP_AMP:
    BIN(boolean, boolean, &&);
  case tBAR_BAR:
    BIN(boolean, boolean, ||);
  default:
    panicf("Unknown binary operator: `%s`", token_display_name(binary->op));
  }
  printf("%zd %s %zd -> %zd\n", left_value.integer,
         token_display_name(binary->op), right_value.integer, value.integer);
  OK(value);
});

EVALUATOR(function_call, {
  EVALUATE(function_call->function, function);

  if (function_value.builtin_id != NOT_BUILTIN) {
    OK(evaluate_builtin(function_call->args, function_value.builtin_id,
                        recursion_depth + 1, stack_frame));
  }
  assert(function_value.function->kind == aFUNCTION);
  __auto_type function = &function_value.function->function;
  Value new_stack_frame[function->local_count];
  memset(new_stack_frame, 121, sizeof(new_stack_frame));
  printf("function call (%zu locals, %zu params, stack frame at %p)\n",
         function->local_count, function->params.length, new_stack_frame);

  ITER_ARRAY(function_call->args, arg, {
    new_stack_frame[i] = evaluate_expr(arg, recursion_depth + 1, stack_frame);
    printf("set param %zu at %p to %zd\n", i, &new_stack_frame[i],
           new_stack_frame[i].integer);
  });
  // assumes that parameter values have been set
  Value function_out = {0};
  evaluate_stmts(function->stmts, recursion_depth + 1, new_stack_frame,
                 &function_out);
  OK(function_out);
});

EVALUATOR(function, {OK((Value){.function = node})});

EVALUATOR(_struct, {OK((Value){._struct = _struct->id})});

#define ECASE($name)                                                           \
  CASE($name, return evaluate_##$name(node, recursion_depth, stack_frame))

Value evaluate_expr(Node *node, size_t recursion_depth, Value *stack_frame) {
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
  default:
    panicf("not an expression: %s", ast_node_name(node->kind));
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
      Value stack_frame[function->local_count];
      Value function_out = {0};
      evaluate_stmts(function->stmts, 0, stack_frame, &function_out);
      return;
    }
  });
  eprintf("`main` function not found.\n");
}
