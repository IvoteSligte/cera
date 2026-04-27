
#include "evaluator.h"
#include "analyzer_shared.h"
#include "ast_macro.h"

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
  SWITCH(node, panicf("not a target: %s", ast_node_name(node->kind)),
         { CASE(name, { return name->value_ptr; }); });
}

ControlFlow evaluate_stmt(Node *node, Value *function_out) {
  SWITCH(
      node,
      {
        evaluate_expr(node);
        OK(cNEXT);
      },
      {
        CASE(param);
        CASE(for_loop, {
          EVALUATE(for_loop->init, init);
          while (true) {
            EVALUATE(for_loop->cond, cond);
            if (!cond_value.boolean)
              break;
            ControlFlow control = evaluate_stmts(for_loop->stmts, function_out);
            if (control == cRETURN)
              OK(cRETURN);
            EVALUATE(for_loop->step, step);
          }
          OK(cNEXT);
        });
        CASE(assign, {
          Value *target = evaluate_target(assign->target);
          EVALUATE(assign->expr, expr);
          *target = expr_value;
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
        CASE(module);
      });
  panicf("not a statement: %s", ast_node_name(node->kind));
}
#undef OK

#define OK(value...) return value;

#define OK_INT($integer) OK((Value){.integer = $integer})

Value evaluate_builtin(NodeArray args, BuiltinID id) {
  switch (id) {
  case NOT_BUILTIN:
    panicf("unreachable");
  case PRINT_STRING: {
    assert(args.length == 1);
    EVALUATE(args.data[0], arg);
    // fwrite instead of printf because printf can only print non-zero-delimited
    // strings of up to INT_MAX characters in length
    fwrite(arg_value.string.text, arg_value.string.length, 1, stdout);
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

EVALUATOR(integer, { OK_INT(integer->value); });

EVALUATOR(string, {
  // the string value is immutable, only typed as mutable as it
  // is normally owned
  OK((Value){
      .string = {.text = (char *)string->text, .length = string->length}});
});

EVALUATOR(unary, {
  EVALUATE(unary->expr, expr);
  if (unary->op == tMINUS) {
    OK_INT(-expr_value.integer)
  }
  panicf("Unknown unary operator: `%s`", token_display_name(unary->op));
});

EVALUATOR(binary, {
  EVALUATE(binary->left, left);
  EVALUATE(binary->right, right);
  ssize_t value = 0;
  switch (binary->op) {
  case tPLUS:
    value = left_value.integer + right_value.integer;
    break;
  case tMINUS:
    value = left_value.integer - right_value.integer;
    break;
  case tSTAR:
    value = left_value.integer * right_value.integer;
    break;
  case tSLASH:
    value = left_value.integer / right_value.integer;
    break;
  default:
    panicf("Unknown binary operator: `%s`", token_display_name(binary->op));
  }
  OK_INT(value);
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
  SWITCH(node, panicf("not an expression: %s", ast_node_name(node->kind)), {
    ECASE(name);
    ECASE(integer);
    ECASE(string);
    ECASE(unary);
    ECASE(binary);
    ECASE(function_call);
    ECASE(function);
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
