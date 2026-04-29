
#include "evaluator.h"
#include "analyzer_shared.h"
#include "ast_macro.h"

#ifdef TEST
extern void print_string(const char *text, size_t length);
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
  Value name##_value = evaluate_expr(node);                                    \
  UNUSED(name##_value);

#define EVALUATE_ARRAY($array)                                                 \
  ITER_ARRAY($array, element, { EVALUATE(element, element); });

ControlFlow evaluate_stmt(Node *node, Value *function_out);

ControlFlow evaluate_stmts(NodeArray stmts, Value *function_out) {
  ITER_ARRAY(stmts, stmt_node, {
    ControlFlow control = evaluate_stmt(stmt_node, function_out);
    if (control == cRETURN)
      return cRETURN;
  });
  return cNEXT;
}

Value *evaluate_target(Node *node) {
  SWITCH(node, {
    CASE(name, { return name->value_ptr; });
  default:
    panicf("not a target: %s", ast_node_name(node->kind));
  });
}

ControlFlow evaluate_stmt(Node *node, Value *function_out) {
  SWITCH(node, {
    CASE(if_stmt, {
      EVALUATE(if_stmt->cond, cond);
      if (cond_value.boolean) {
        ControlFlow control = evaluate_stmts(if_stmt->stmts, function_out);
        OK(control);
      }
      OK(cNEXT);
    });
    CASE(while_loop, {
      while (true) {
        EVALUATE(while_loop->cond, cond);
        if (!cond_value.boolean)
          break;
        ControlFlow control = evaluate_stmts(while_loop->stmts, function_out);
        if (control == cRETURN)
          OK(cRETURN);
      }
      OK(cNEXT);
    });
    CASE(for_loop, {
      evaluate_stmt(for_loop->init, function_out);
      while (true) {
        EVALUATE(for_loop->cond, cond);
        if (!cond_value.boolean)
          break;
        ControlFlow control = evaluate_stmts(for_loop->stmts, function_out);
        if (control == cRETURN)
          OK(cRETURN);
        evaluate_stmt(for_loop->step, function_out);
      }
      OK(cNEXT);
    });
    CASE(assign, {
      Value *target = evaluate_target(assign->target);
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
    CASE(return_stmt, {
      EVALUATE(return_stmt->expr, expr);
      *function_out = expr_value;
      OK(cRETURN);
    });
    CASE(declaration, {
      if (!declaration->is_constant) {
        EVALUATE(declaration->expr, value);
        declaration->symbol_data->value = value_value;
      }
      OK(cNEXT);
    });
  default:
    evaluate_expr(node);
    OK(cNEXT);
  });
}
#undef OK

#define OK(value...) return value;

Value evaluate_builtin(NodeArray args, BuiltinID id) {
  switch (id) {
  case NOT_BUILTIN:
    panicf("unreachable");
  case PRINT_STRING: {
    assert(args.length == 1);
    EVALUATE(args.data[0], arg);
#ifdef TEST
    print_string(arg_value.string.text, arg_value.string.length);
#else
    // Using fwrite instead of printf because printf can only print non-zero-delimited
    // strings of up to INT_MAX characters in length.
    fwrite(arg_value.string.text, arg_value.string.length, 1, stdout);
#endif    
    return (Value){0};
  }
  default:
    panicf("unimplemented builtin: %d", id);
  }
}

#define EVALUATOR($name, ...)                                                  \
  Value evaluate_##$name(Node *node) {                                         \
    __auto_type $name = &node->$name;                                          \
    UNUSED($name);                                                             \
    __VA_ARGS__;                                                               \
  }

EVALUATOR(name, { OK(*name->value_ptr); });

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
  OK(value);
});

EVALUATOR(function_call, {
  EVALUATE(function_call->function, function);

  if (function_value.builtin_id != NOT_BUILTIN) {
    OK(evaluate_builtin(function_call->args, function_value.builtin_id));
  }

  assert(function_value.function->kind == aFUNCTION);
  __auto_type function = &function_value.function->function;

  ITER_ARRAY(function_call->args, arg, {
    Node *param = function->params.data[i];
    param->param.symbol_data->value = evaluate_expr(arg);
  });
  // assumes that parameter values have been set
  Value function_out = {0};
  evaluate_stmts(function->stmts, &function_out);
  OK(function_out);
});

EVALUATOR(function, {OK((Value){.function = node})});

#define ECASE($name) CASE($name, return evaluate_##$name(node))

Value evaluate_expr(Node *node) {
  SWITCH(node, {
    ECASE(name);
    ECASE(integer);
    ECASE(boolean);
    ECASE(string);
    ECASE(unary);
    ECASE(binary);
    ECASE(function_call);
    ECASE(function);
  default:
    panicf("not an expression: %s", ast_node_name(node->kind));
  });
}

static const Name MAIN_NAME = (Name){.text = "main", .length = 4};

void evaluate_module(Node *node) {
  assert(node->kind == aMODULE);
  __auto_type module = &node->module;
  ITER_ARRAY(module->declarations, declaration_node, {
    __auto_type declaration = &declaration_node->declaration;
    if (name_eq(declaration->name->name.name, MAIN_NAME)) {
      // FIXME: this evaluates the declaration, not the function in the
      // declaration
      // there is also no check that `main` has the correct signature
      evaluate_stmt(declaration_node, NULL);

      assert(declaration->expr->kind == aFUNCTION);
      __auto_type function = &declaration->expr->function;
      Value function_out = {0};
      evaluate_stmts(function->stmts, &function_out);
      return;
    }
  });
  eprintf("`main` function not found.\n");
}
