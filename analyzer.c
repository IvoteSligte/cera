
#include "analyzer.h"
#include "analyzer_shared.h"
#include "ast.h"
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
    panicf("Failed to ssprintf.");
  return out;
}

#define NAME_OF($declaration)                                                  \
  assert(($declaration)->name->kind == aNAME);                                 \
  Name name = ($declaration)->name->name.name;

#define FAIL                                                                   \
  {                                                                            \
    return false;                                                              \
  }

#define ANALYZE_NODE($node, $name, $out_type)                                  \
  analyze_node(allocator, $node, table, return_type, $out_type, error_data,    \
               is_static)

#define ANALYZE($node, $name)                                                  \
  Type $name##_type = {0};                                                     \
  if (!ANALYZE_NODE($node, $name, &$name##_type))                              \
    FAIL;

#define ANALYZE_TYPE($node, $name)                                             \
  ANALYZE($node, $name);                                                       \
  EXPECT(($name##_type.kind == tyTYPE), $node, strdup("not a type"))

#define ANALYZE_ARRAY($array, $table)                                          \
  ITER_ARRAY($array, element, {                                                \
    Table *table = $table;                                                     \
    ANALYZE(element, element);                                                 \
  });

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
  rDONE = 0,
  rERROR,
  rBLOCKED,
} Result;

#define OK                                                                     \
  {                                                                            \
    node->stage = sANALYZED;                                                   \
    return true;                                                               \
  }

#define IS_GLOBAL (table->parent == NULL)

#define PRIM(type_kind)                                                        \
  (Type) { .kind = type_kind, .name = {0} }

Result analyze_node(Allocator *allocator, Node *node, Table *table,
                    Type return_type, Type *out_type,
                    TypeErrorArray *error_data, bool is_static) {
  if (node->stage >= sANALYZED)
    return rDONE;
  SWITCH(
      node,
      panicf("analyze not implemented for node: %s", ast_node_name(node->kind)),
      {
        CASE(name, {
          SymbolData *symbol_data = NULL;
          if (!get_symbol(table, name->name, &symbol_data)) {
            return rBLOCKED;
          }
          EXPECT((!is_static || symbol_data->is_static), node,
                 strdup("static declaration refers to runtime variable"));
          name->value_ptr = &symbol_data->value;
          *out_type = symbol_data->type;
          OK;
        });
        CASE(integer, {
          EXPECT((sscanf(integer->text, "%zd", &integer->value) == 1), node,
                 strdup("integer is too large"));
          *out_type = PRIM(tyINT);
          OK;
        });
        CASE(string, {
          *out_type = PRIM(tySTRING);
          OK;
        });
        CASE(unary, {
          ANALYZE(unary->expr, expr);
          if (unary->op == tMINUS) {
            EXPECT(
                (expr_type.kind != tyINT), unary->expr,
                strdup("cannot apply unary operator `-` to non-numeric type"));
            *out_type = expr_type;
            OK;
          }
          panicf("Unknown unary operator: `%s`", token_display_name(unary->op));
        });
        CASE(binary, {
          ANALYZE(binary->left, left);
          ANALYZE(binary->right, right);
          if (IS_ONE_OF(binary->op, tPLUS, tMINUS, tSTAR, tSLASH)) {
            EXPECT(
                (left_type.kind == tyINT), binary->left,
                strdup("cannot apply arithmetic operator to non-numeric type"));
            EXPECT(
                (right_type.kind == tyINT), binary->right,
                strdup("cannot apply arithmetic operator to non-numeric type"));
            EXPECT(
                (left_type.kind == right_type.kind), node,
                strdup("types on both sides of the binary arithmetic operator "
                       "must be the same"));
            *out_type = left_type;
            OK;
          }
          panicf("Unknown binary operator: `%s`",
                 token_display_name(binary->op));
        });
        CASE(function_call, {
          EXPECT((!is_static), node,
                 strdup("cannot call function in static code"));
          ANALYZE(function_call->function, function);
          EXPECT((function_type.kind == tyFUNCTION), function_call->function,
                 strdup("not a function"));
          {
            __auto_type function = function_type.function;
            if (function._return != NULL) {
              *out_type = *function._return;
            }

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
          }
          OK;
        });
        CASE(function, {
          EXPECT((!IS_GLOBAL), node,
                 strdup("functions can only be defined in the global scope"));

          if (node->stage < sTYPED) {
            function->table =
                (Table){.parent = table, .data = NULL, .length = 0};
            Table *table = &function->table;
            *out_type = (Type){.kind = tyFUNCTION, .function = {0}};
            out_type->function.params =
                ra_calloc(allocator, sizeof(Type) * function->num_params);
            ITER_ARRAY(function->params, param, {
              ANALYZE(param, param);
              out_type->function.params[i] = param_type;
            });
            if (function->return_type != NULL) {
              ANALYZE_TYPE(function->return_type, declared);
              out_type->function._return = ra_calloc(allocator, sizeof(Type));
              *out_type->function._return = declared_type;
            }
            node->stage = sTYPED;
          }
          ANALYZE_ARRAY(function->stmts, &function->table);
          OK;
        });
        CASE(param, {
          ANALYZE_TYPE(param->type, param);
          NAME_OF(param);
          SymbolData *symbol_data = NULL;
          EXPECT(add_symbol(allocator, table, name, &symbol_data), node,
                 strdup("duplicate parameter"));
          symbol_data->type = param_type;
          param->value_ptr = &symbol_data->value;
          OK;
        });
        CASE(for_loop, {
          ANALYZE(for_loop->init, init);
          ANALYZE(for_loop->cond, cond);
          ANALYZE(for_loop->step, step);
          EXPECT((cond_type.kind == tyBOOL), node,
                 strdup("expected boolean type for for-loop condition"));
          for_loop->table.parent = table;

          ANALYZE_ARRAY(for_loop->stmts, &for_loop->table);
          OK;
        });
        CASE(assign, {
          ANALYZE(assign->target, target);
          ANALYZE(assign->expr, expr);
          EXPECT((type_eq(target_type, expr_type)), node,
                 strdup("type mismatch"));
          OK;
        });
        CASE(return_stmt, {
          ANALYZE(return_stmt->expr, expr);
          EXPECT(type_eq(expr_type, return_type), return_stmt->expr,
                 strdup("return type mismatch"));
          OK;
        });
        CASE(declaration, {
          NAME_OF(declaration);
          bool is_static = IS_GLOBAL || declaration->is_constant;

          SymbolData *symbol_data = NULL;
          EXPECT(add_symbol(allocator, table, name, &symbol_data), node,
                 strdup("duplicate declaration"));
          symbol_data->is_static = is_static;
          declaration->value_ptr = &symbol_data->value;

          if (!ANALYZE_NODE(declaration->expr, value, &symbol_data->type))
            FAIL;
          if (is_static) {
            symbol_data->value = evaluate_expr(declaration->expr);
          }
          OK;
        });
        CASE(module, {
          ANALYZE_ARRAY(module->declarations, &module->table);
          OK;
        });
      });
}

bool analyze(AST *ast, TypeErrorArray *error_data) {
  Allocator allocator = {0};
  Table table = {0};
  Result result = 0;
  while (true) {
    Result result = analyze_node(&allocator, ast->head, &table, (Type){0}, NULL,
                                 error_data, true);
    if (result == rERROR)
      break;
  }
  assert(result != rBLOCKED); // should be finished after 2 passes
  free(table.data);
  // NOTE: this leaves the type in an undefined state
  // should the allocator be passed along without freeing
  // or should types be zeroed?
  ra_free_all(&allocator);
  return result == rDONE;
}
