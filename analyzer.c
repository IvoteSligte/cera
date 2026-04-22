
#include "analyzer.h"
#include "analyzer_shared.h"
#include "ast_macro.h"
#include "evaluator.h"

#include <stdarg.h>
#include <string.h>

// Creates a string like sprintf, but panics on failure.
char *ssprintf(const char *fmt, ...) {
  va_list args;
  va_start(args, fmt);
  char *out = NULL;
  // TODO: non-GNU alternative to asprintf
  if (vasprintf(&out, fmt, args) < 0)
    panicf("Failed to ssprintf.") return out;
}

#define FAIL                                                                   \
  {                                                                            \
    return false;                                                              \
  }

#define ANALYZE(node, name)                                                    \
  Type name##_type = {0};                                                      \
  if (!analyze_node(allocator, node, table, return_type, &name##_type,         \
                    error_data, is_static, is_second_pass))                    \
    FAIL;

#define ANALYZE_TYPE(node, name)                                               \
  ANALYZE(node, name);                                                         \
  EXPECT((name##_type.kind == tyTYPE), node, strdup("not a type"))

#define ANALYZE_ARRAY(array)                                                   \
  ITER_ARRAY(array, element, { ANALYZE(element, element); });

void add_error(TypeErrorArray *error_data, Span span, char *message) {
  TypeError error = {.span = span, .message = message};
  error_data->data =
      realloc(error_data->data, sizeof(TypeError) * (error_data->length + 1));
  error_data->data[error_data->length] = error;
  error_data->length++;
}

#define EXPECT(condition, node, $message)                                      \
  if (!(condition)) {                                                          \
    add_error(error_data, node->span, $message);                               \
    return false;                                                              \
  }

typedef enum {
  rERROR,
  rBLOCKED,
  rOK,
} Result;

// explicitly takes 0 parameters to prevent confusion with OK_EXPR
#define OK()                                                                   \
  {                                                                            \
    node->is_analyzed = true;                                                  \
    return true;                                                               \
  }
#define OK_EXPR($type...)                                                      \
  {                                                                            \
    *out_type = $type;                                                         \
    node->is_analyzed = true;                                                  \
    return true;                                                               \
  }

#define IS_GLOBAL (table->parent == NULL)

#define PRIM(type_kind)                                                        \
  (Type) { .kind = type_kind, .name = {0} }

Result analyze_node(Allocator *allocator, Node *node, Table *table,
                    Type return_type, Type *out_type,
                    TypeErrorArray *error_data, bool is_static,
                    bool is_second_pass) {
  if (node->is_analyzed)
    return rOK;
  SWITCH(node, {
    CASE(name, {
      Symbol symbol = {0};
      if (!get_symbol(table, name->name, &symbol)) {
        if (is_second_pass) {
          add_error(error_data, node->span, strdup("undefined variable"));
          FAIL;
        } else {
          return rBLOCKED;
        }
      }
      name->target = symbol.value_ptr;
      OK_EXPR(symbol.type);
    });
    CASE(integer, {
      EXPECT((sscanf(integer->text, "%zd", &integer->value) == 1), node,
             strdup("integer is too large"));
      OK_EXPR(PRIM(tyINT));
    });
    CASE(string, { OK_EXPR(PRIM(tySTRING)); });
    CASE(unary, {
      ANALYZE(unary->expr, expr);
      if (unary->op == tMINUS) {
        EXPECT((expr_type.kind != tyINT), unary->expr,
               strdup("cannot apply unary operator `-` to non-numeric type"));
        OK_EXPR(expr_type);
      }
      panicf("Unknown unary operator: `%s`", token_display_name(unary->op));
    });
    CASE(binary, {
      ANALYZE(binary->left, left);
      ANALYZE(binary->right, right);
      if (IS_ONE_OF(binary->op, tPLUS, tMINUS, tSTAR, tSLASH)) {
        EXPECT((left_type.kind == tyINT), binary->left,
               strdup("cannot apply arithmetic operator to non-numeric type"));
        EXPECT((right_type.kind == tyINT), binary->right,
               strdup("cannot apply arithmetic operator to non-numeric type"));
        EXPECT((left_type.kind == right_type.kind), node,
               strdup("types on both sides of the binary arithmetic operator "
                      "must be the same"));
        OK_EXPR(left_type);
      }
      panicf("Unknown binary operator: `%s`", token_display_name(binary->op));
    });
    CASE(function_call, {
      EXPECT((!is_static), node, strdup("cannot call function in static code"));
      ANALYZE(function_call->function, function);
      EXPECT((function_type.kind == tyFUNCTION), function_call->function,
             strdup("not a function"));
      Type function_return_type = {0};
      {
        __auto_type function = function_type.function;

        Node *arg = function_call->args;
        Type *param_type = function.params;
        size_t i = 0;
        while (arg != NULL || i < function.num_params) {
          ANALYZE(arg, arg);
          EXPECT((arg != NULL && param_type != NULL), node,
                 strdup("argument count mismatch"));
          EXPECT((type_eq(arg_type, *param_type)), arg,
                 strdup("argument type mismatch"));
          arg = arg->next_sibling;
          i++;
        }
        if (function._return != NULL) {
          function_return_type = *function._return;
        }
      }
      OK_EXPR(function_return_type);
    });
    CASE(function, {
      EXPECT((!IS_GLOBAL), node,
             strdup("functions can only be defined in the global scope"));
      // function params and return_type are analyzed by declaration
      Table *table = &function->table;
      ANALYZE_ARRAY(function->stmts);
      OK();
    });
    CASE(param, {
      ANALYZE_TYPE(param->type, type);
      OK_EXPR(type_type);
    });
    CASE(for_loop, {
      panicf("TODO");
      OK();
    });
    CASE(assign, {
      panicf("TODO");
      OK();
    });
    CASE(return_stmt, {
      ANALYZE(return_stmt->expr, expr);
      EXPECT(type_eq(expr_type, return_type), return_stmt->expr,
             strdup("return type mismatch"));
      OK();
    });
    CASE(declaration, {
      assert(declaration->name->kind == NAME);
      Name name = declaration->name->name.name;
      bool is_static = IS_GLOBAL || declaration->is_constant;
      Value *value_ptr = ra_calloc(allocator, sizeof(Value));

      if (declaration->expr->kind == FUNCTION) {
        // a function's type needs to be determined before its
        // body is analyzed to prevent issues with recursion
        __auto_type function = &declaration->expr->function;
        function->table = (Table){.parent = table, .data = NULL, .length = 0};
        Table *table = &function->table;
        Type type = {.kind = tyFUNCTION, .function = {0}};
        EXPECT((function->num_params < MAX_NUM_PARAMS), declaration->expr,
               strdup("functions may only have " STRINGIFY(
                   MAX_NUM_PARAMS) " parameters"));
        type.function.params =
            ra_calloc(allocator, sizeof(Type) * function->num_params);
        ITER_ARRAY(function->params, param, {
          ANALYZE(param, param);
          type.function.params[i] = param_type;
        });
        if (function->return_type != NULL) {
          ANALYZE_TYPE(function->return_type, declared);
          type.function._return = ra_calloc(allocator, sizeof(Type));
          *type.function._return = declared_type;
        }

        EXPECT(add_symbol(allocator, table, name, type, value_ptr, true), node,
               strdup("duplicate declaration"));
        // NOTE: should the function analysis just be inlined here?
        ANALYZE(declaration->expr, value); // uses is_static override
      } else {
        ANALYZE(declaration->expr, value); // uses is_static override
        EXPECT(add_symbol(allocator, table, name, value_type, value_ptr,
                          is_static),
               node, strdup("duplicate declaration"));
      }
      if (is_static) {
        *value_ptr = evaluate_expr(declaration->expr);
      }
      // TODO: check if this is a constant that refers to a runtime declaration
      // (in a function)
      OK();
    });
    CASE(module, {
      Table *table = &module->table;
      ANALYZE_ARRAY(module->declarations);
      OK();
    });
  });
}

bool analyze(AST *ast, TypeErrorArray *error_data) {
  Allocator allocator = {0};
  Table table = {0};
  Result result = 0;
  // needs two passes so that forward references are also resolved
  for (bool is_second_pass = false; !is_second_pass; is_second_pass = true) {
    Result result = analyze_node(&allocator, ast->head, &table, (Type){0}, NULL,
                                 error_data, true, is_second_pass);
    if (result == rERROR)
      break;
  }
  assert(result != rBLOCKED); // should be finished after 2 passes
  free(table.data);
  // NOTE: this leaves the type in an undefined state
  // should the allocator be passed along without freeing
  // or should types be zeroed?
  ra_free_all(&allocator);
  return result == rOK;
}
