
#include "evaluator.h"
#include "analyzer_shared.h"
#include "ast_macro.h"

typedef enum {
  cNEXT = 0,
  cRETURN,
} ControlFlow;

#define OK(control_flow)                                                       \
  {                                                                            \
    return control_flow;                                                       \
  }

#define EVALUATE(node, name) Value name##_value = evaluate_expr(node);

ControlFlow evaluate_stmt(Node *node, Value *function_out) {
  SWITCH(node, {
    CASE(name);
    CASE(integer);
    CASE(string);
    CASE(unary);
    CASE(binary);
    CASE(function_call);
    CASE(function);
    CASE(param);
    CASE(for_loop, {
      panicf("TODO");
      OK(cNEXT);
    });
    CASE(assign, {
      panicf("TODO");
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
        node->value = value_value;
      }
      OK(cNEXT);
    });
    CASE(module);
  });
  panicf("not a statement: %s", ast_node_name(node->kind));
}
#undef OK

#define OK(value...)                                                           \
  {                                                                            \
    return value;                                                              \
  }
#define OK_INT($integer) OK((Value){.integer = $integer})

Value evaluate_expr(Node *node) {
  SWITCH(node, {
    CASE(name, { OK(*name->target); });
    CASE(integer, { OK_INT(integer->value); });
    CASE(string, {
      // the string value is immutable, only typed as mutable as it
      // is normally owned
      OK((Value){
          .string = {.text = (char *)string->text, .length = string->length}});
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
      EVALUATE(function_call->function, function);
      assert(function_value.function->kind == FUNCTION);
      __auto_type function = &function_value.function->function;

      ASTNode *arg = function_call->args;
      ASTNode *param = function->params;
      while (arg != NULL) {
        param->value = evaluate_expr(arg);
        arg = arg->next_sibling;
        param = param->next_sibling;
      }
      OK(evaluate_expr(function_value.function));
    });
    CASE(function, {
      // assumes that parameter values have been set
      Value function_out = {0};
      ITER_ARRAY(function->stmts, stmt, {
        ControlFlow control = evaluate_stmt(stmt, &function_out);
        switch (control) {
        case cNEXT:
          break; // break switch
        case cRETURN:
          OK(function_out);
        }
      });
    });
    CASE(param);
    CASE(for_loop);
    CASE(assign);
    CASE(return_stmt);
    CASE(declaration);
    CASE(module);
  });
  panicf("not an expression: %s", ast_node_name(node->kind));
}

void evaluate_module(Node *node) {
  assert(node->kind == MODULE);
  __auto_type module = &node->module;
  ITER_ARRAY(module->declarations, declaration,
             { evaluate_stmt(declaration, NULL); });
}
