
#include "analyzer_shared.h"
#include "ast_macro.h"

typedef enum {
  cNEXT = 0,
  cRETURN,
} ControlFlow;

#define EVALUATE(node, name)                                                   \
  Value name##_value = {0};                                                    \
  ControlFlow name##_control_flow = evaluate_node(node, &name##_value);

#define EVALUATOR_SIGNATURE

#define OK                                                                     \
  {                                                                            \
    node->stage = EVALUATED;                                                   \
    return;                                                                    \
  }

void evaluate_global_node(Node *node) {
  assert(node->stage >= TYPED);
  SWITCH(node, {
    CASE(declaration, { // TODO
      OK;
    });
    CASE(function, { OK; });
  } default:);
  panicf("not a global node kind: %s", ast_node_name(node->kind));
}
#undef OK

#define OK(control_flow, value...)                                             \
  {                                                                            \
    node->stage = EVALUATED;                                                   \
    *out = value;                                                              \
    return control_flow;                                                       \
  }
#define OK_VOID OK(cNEXT, (Value){0})
#define OK_INT($integer) OK(cNEXT, (Value){.integer = $integer})
#define OK_STR($text, $length)                                                 \
  OK(cNEXT, (Value){.string = {.text = $text, .length = $length}})

ControlFlow evaluate_node(Node *node, Value *out);

void evaluate_function(Node *function_node, Value *function_args, Value *out) {
  assert(function_node->kind == FUNCTION);
  __auto_type function = &function_node->function;

  ITER_ARRAY(function->params, param, { param->value = function_args[i]; });
  ITER_ARRAY(function->stmts, stmt, {
    EVALUATE(stmt, stmt);
    switch (stmt_control_flow) {
    case cNEXT:
      break; // break switch
    case cRETURN:
      return;
    }
  });
}

ControlFlow evaluate_node(Node *node, Value *out) {
  assert(node->stage >= TYPED);
  SWITCH(node, {
    CASE(name, { OK(cNEXT, *name->target); });
    CASE(integer, { OK_INT(integer->value); });
    CASE(string, {
      // the string value is immutable, only typed as mutable as it
      // is normally owned
      OK_STR((char *)string->text, string->length);
    });
    CASE(unary, {
      EVALUATE(unary->expr, expr);
      if (unary->op == tMINUS) {
        OK_INT(-expr_value.integer)
      }
      panicf("Unknown unary operator: `%s`", token_display_name(unary->op));
    });
    CASE(binary, {
      EVALUATE(binary->left, left);
      EVALUATE(binary->right, right);
      ssize_t value = 0;
      switch (binary->op) {
      case tPLUS:
        value = left_value.integer + right_value.integer;
      case tMINUS:
        value = left_value.integer - right_value.integer;
      case tSTAR:
        value = left_value.integer * right_value.integer;
      case tSLASH:
        value = left_value.integer / right_value.integer;
      default:
        panicf("Unknown binary operator: `%s`", token_display_name(binary->op));
      }
      OK_INT(value);
    });
    CASE(function_call, {
      Value function_args[MAX_NUM_PARAMS] = {0};
      ITER_ARRAY(function_call->args, arg,
                 { evaluate_node(arg, &function_args[i]); });
      evaluate_function(function_call->function, function_args, out);
      OK(cNEXT, (*out));
    });
    CASE(function, { panicf("local functions should not exist"); });
    CASE(param, { panicf("param should not be evaluated"); });
    CASE(for_loop, {
      panicf("TODO");
      OK_VOID;
    });
    CASE(assign, {
      panicf("TODO");
      OK_VOID;
    });
    CASE(return_stmt, {
      EVALUATE(return_stmt->expr, expr);
      OK(cRETURN, expr_value);
    });
    CASE(declaration, {
      if (!declaration->is_constant) {
        EVALUATE(declaration->expr, value);
        node->value = value_value;
      }
      OK_VOID;
    });
    CASE(module, {
      ITER_ARRAY(module->declarations, declaration,
                 { evaluate_global_node(declaration); });
      OK_VOID;
    });
  });
}
