
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

#define RETURN(control_flow)                                                   \
  {                                                                            \
    return control_flow;                                                       \
  }

#define EVALUATE($node, $name)                                                 \
  Value $name = evaluate_expr($node, recursion_depth, stack_frame);

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
  EVALUATE(if_stmt->cond, cond_value);
  ControlFlow control = 0;
  if (cond_value._bool) {
    control = EVALUATE_STMTS(if_stmt->then_stmts);
  } else {
    control = EVALUATE_STMTS(if_stmt->else_stmts);
  }
  RETURN(control);
});

EVALUATOR(while_loop, {
  while (true) {
    EVALUATE(while_loop->cond, cond_value);
    if (!cond_value._bool)
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
    if (!cond_value._bool)
      break;
    ControlFlow control = EVALUATE_STMTS(for_loop->stmts);
    if (control == cRETURN)
      RETURN(cRETURN);
    EVALUATE_STMT(for_loop->step);
  }
  RETURN(cNEXT);
});

#define BIN_ASSIGN($op) target->_int $op expr_value._int

EVALUATOR(assign, {
  Value *target = evaluate_target(assign->target, stack_frame);
  EVALUATE(assign->expr, expr_value);
  switch (assign->op) {
  case tEQ:
    *target = expr_value;
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
  printf("returning %zd\n", expr_value._int);
  *function_out = expr_value;
  RETURN(cRETURN);
});

EVALUATOR(decl, {
  if (!decl->is_constant) {
    EVALUATE(decl->expr, value);
    stack_frame[decl->local_index] = value;
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
    evaluate_expr(node, recursion_depth, stack_frame);
    RETURN(cNEXT);
  }
  });
}
#undef PARAMS
#undef RETURN
#undef EVALUATOR
#undef ECASE

Value evaluate_builtin(NodeArray args, BuiltinID id, size_t recursion_depth,
                       Value *stack_frame) {
  switch (id) {
  case NOT_BUILTIN:
    panicf("unreachable");
  case PRINT_STRING: {
    assert(args.length == 1);
    EVALUATE(args.data[0], arg_value);
#ifdef TEST
    print_string(arg_value.string.text, arg_value.string.length);
#else
    // Using fwrite instead of printf because printf can only print
    // non-zero-delimited strings of up to INT_MAX characters in length.
    String arg_string = arg_value.string;
    fwrite(arg_string.text, arg_string.length, 1, stdout);
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

#define OK return

#define RETURN($value...) return $value;

#define STACK_GET($type, $offset) ($type *)(&stack_frame[$offset])
#define STACK_INT($offset) *STACK_GET(Int, $offset)

EVALUATOR(name, {
  if (name->is_static) {
    printf("static `%.*s`: %zd\n", FMT(name->name),
           name->static_value_ptr->_int);
    RETURN(*name->static_value_ptr);
  } else {
    printf("local %zu `%.*s` at %p: %zd\n", name->local_index, FMT(name->name),
           &stack_frame[name->local_index],
           stack_frame[name->local_index]._int);
    RETURN(stack_frame[name->local_index]);
  }
});

EVALUATOR(integer, { RETURN((Value){._int = integer->value}); });

EVALUATOR(boolean, { RETURN((Value){._bool = boolean->value}); });

EVALUATOR(string, { RETURN((Value){.string = string->value}); });

EVALUATOR(unary, {
  EVALUATE(unary->expr, expr_value);
  if (unary->op == tMINUS) {
    RETURN((Value){._int = -expr_value._int});
  }
  panicf("Unknown unary operator: `%s`", token_display_name(unary->op));
});

#define BIN($out_member, $in_member, $op)                                      \
  value.$out_member = left_value.$in_member $op right_value.$in_member;        \
  break;

EVALUATOR(binary, {
  EVALUATE(binary->left, left_value);
  EVALUATE(binary->right, right_value);
  Value value = {0};
  switch (binary->op) {
  case tPLUS:
    BIN(_int, _int, +);
  case tMINUS:
    BIN(_int, _int, -);
  case tSTAR:
    BIN(_int, _int, *);
  case tSLASH:
    BIN(_int, _int, /);
  case tLT:
    BIN(_bool, _int, <);
  case tGT:
    BIN(_bool, _int, >);
  case tLT_EQ:
    BIN(_bool, _int, <=);
  case tGT_EQ:
    BIN(_bool, _int, >=);
  case tEQ_EQ:
    BIN(_bool, _int, ==);
  case tAMP_AMP:
    BIN(_bool, _bool, &&);
  case tBAR_BAR:
    BIN(_bool, _bool, ||);
  default:
    panicf("Unknown binary operator: `%s`", token_display_name(binary->op));
  }
  printf("%zd %s %zd -> %zd\n", left_value._int, token_display_name(binary->op),
         right_value._int, value._int);
  RETURN(value);
});

EVALUATOR(function_call, {
  EVALUATE(function_call->function, function_value);

  if (function_value.builtin_id != NOT_BUILTIN) {
    RETURN(evaluate_builtin(function_call->args, function_value.builtin_id,
                            recursion_depth + 1, stack_frame));
  }
  assert(function_value.function->kind == aFUNCTION);
  __auto_type function = &function_value.function->function;
  // C requires that a variable-sized array contains at least 1 element.
  Value new_stack_frame[MAX(function->frame_length, 1)];
  memset(new_stack_frame, 0, sizeof(new_stack_frame));
  printf("function call (%zu locals, %zu params, stack frame at %p)\n",
         function->frame_length, function->params.length, new_stack_frame);

  ITER_ARRAY(function_call->args, arg, {
    new_stack_frame[i] = evaluate_expr(arg, recursion_depth + 1, stack_frame);
    printf("set param %zu at %p to %zd\n", i, &new_stack_frame[i],
           new_stack_frame[i]._int);
  });
  // assumes that parameter values have been set
  Value function_out = {0};
  evaluate_stmts(function->stmts, recursion_depth + 1, new_stack_frame,
                 &function_out);
  RETURN(function_out);
});

EVALUATOR(function, {
  RETURN((Value){.function = node});
});

/* EVALUATOR(_struct, { RETURN(value); }); */
/*  */
/* EVALUATOR(struct_inst, { */
/*   RETURN((Value) { ._struct = . }); */
/* }); */

#define ECASE($name)                                                           \
  CASE($name,                                                                  \
       return evaluate_##$name(node, recursion_depth, stack_frame))

Value evaluate_expr(ASTNode *node, size_t recursion_depth, Value *stack_frame) {
  SWITCH(node, {
    ECASE(name);
    ECASE(integer);
    ECASE(boolean);
    ECASE(string);
    ECASE(unary);
    ECASE(binary);
    ECASE(function_call);
    ECASE(function);
    /* ECASE(_struct); */
    /* ECASE(struct_inst);     */
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

      Value stack_frame[MAX(function->frame_length, 1)];
      memset(stack_frame, 0, sizeof(stack_frame));

      Value function_out = {0};
      evaluate_stmts(function->stmts, 0, stack_frame, &function_out);
      return;
    }
  });
  eprintf("`main` function not found.\n");
}
