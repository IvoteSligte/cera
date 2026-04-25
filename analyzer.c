
#include "analyzer.h"
#include "analyzer_shared.h"
#include "ast.h"
#include "ast_macro.h"
#include "evaluator.h"
#include "offset.h"

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

#define ACASE($name)                                                           \
  CASE($name, {                                                                \
    return analyze_##$name(allocator, node, table, return_type, out_type,      \
                           error_data, is_static);                             \
  })

#define ANALYZE_NODE($node, $name, $out_type)                                  \
  analyze_node(allocator, $node, table, return_type, $out_type, error_data,    \
               is_static)
#define ANALYZE($node, $name)                                                  \
  Type $name##_type = {0};                                                     \
  {                                                                            \
    Result __result = ANALYZE_NODE($node, $name, &$name##_type);               \
    if (__result != rDONE)                                                     \
      return __result;                                                         \
  }

#define ANALYZE_TYPE($node, $name)                                             \
  ANALYZE($node, $name);                                                       \
  EXPECT(($name##_type.kind == tyTYPE), $node, strdup("not a type"));

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
    return rERROR;                                                             \
  }

typedef enum {
  rDONE = 0,
  rERROR,
  rBLOCKED,
} Result;

#define OK                                                                     \
  {                                                                            \
    node->stage = sANALYZED;                                                   \
    return rDONE;                                                              \
  }

#define IS_GLOBAL (table->parent == NULL)

#define PRIM(type_kind)                                                        \
  (Type) { .kind = type_kind, .name = {0} }

#define ANALYZER_SIGNATURE($name)                                              \
  Result analyze_##$name(Allocator *allocator, Node *node, Table *table,       \
                         Type return_type, Type *out_type,                     \
                         TypeErrorArray *error_data, bool is_static)

#define ANALYZER($name, ...)                                                   \
  ANALYZER_SIGNATURE($name) {                                                  \
    __auto_type $name = &node->$name;                                          \
    __VA_ARGS__;                                                               \
    UNUSED(out_type);                                                          \
    UNUSED(table);                                                             \
    UNUSED(return_type);                                                       \
    UNUSED(error_data);                                                        \
    UNUSED(is_static);                                                         \
    UNUSED(allocator);                                                         \
  }

ANALYZER_SIGNATURE(node);

ANALYZER(name, {
  SymbolData *symbol_data = NULL;
  if (!get_symbol(table, name->name, &symbol_data)) {
    eprintf("INFO: compilation blocked by undefined symbol `%.*s` \n",
            (int)name->name.length, name->name.text);
    return rBLOCKED;
  }
  EXPECT((!is_static || symbol_data->is_static), node,
         strdup("static declaration refers to runtime variable"));
  name->value_ptr = &symbol_data->value;
  *out_type = symbol_data->type;
  OK;
});

ANALYZER(integer, {
  EXPECT((sscanf(integer->text, "%zd", &integer->value) == 1), node,
         strdup("integer is too large"));
  *out_type = PRIM(tyINT);
  OK;
});

ANALYZER(string, {
  UNUSED(string);
  *out_type = PRIM(tySTRING);
  OK;
});

ANALYZER(unary, {
  ANALYZE(unary->expr, expr);
  if (unary->op == tMINUS) {
    EXPECT((expr_type.kind != tyINT), unary->expr,
           strdup("cannot apply unary operator `-` to non-numeric type"));
    *out_type = expr_type;
    OK;
  }
  panicf("Unknown unary operator: `%s`", token_display_name(unary->op));
});

ANALYZER(binary, {
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
    *out_type = left_type;
    OK;
  }
  panicf("Unknown binary operator: `%s`", token_display_name(binary->op));
});

ANALYZER(function_call, {
  EXPECT((!is_static), node, strdup("cannot call function in static code"));
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

ANALYZER(function, {
  EXPECT(IS_GLOBAL, node,
         strdup("functions can only be defined in the global scope"));

  if (node->stage < sTYPED) {
    function->table = (Table){.parent = table, .data = NULL, .length = 0};
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
  bool is_static = false;
  ANALYZE_ARRAY(function->stmts, &function->table);
  OK;
});

ANALYZER(param, {
  ANALYZE_TYPE(param->type, param);
  NAME_OF(param);
  SymbolData *symbol_data = NULL;
  EXPECT(add_symbol(allocator, table, name, &symbol_data), node,
         strdup("duplicate parameter"));
  symbol_data->type = param_type;
  param->value_ptr = &symbol_data->value;
  OK;
});

ANALYZER(for_loop, {
  ANALYZE(for_loop->init, init);
  ANALYZE(for_loop->cond, cond);
  ANALYZE(for_loop->step, step);
  EXPECT((cond_type.kind == tyBOOL), node,
         strdup("expected boolean type for for-loop condition"));
  for_loop->table.parent = table;

  ANALYZE_ARRAY(for_loop->stmts, &for_loop->table);
  OK;
});

ANALYZER(assign, {
  ANALYZE(assign->target, target);
  ANALYZE(assign->expr, expr);
  EXPECT((type_eq(target_type, expr_type)), node, strdup("type mismatch"));
  OK;
});

ANALYZER(return_stmt, {
  ANALYZE(return_stmt->expr, expr);
  EXPECT(type_eq(expr_type, return_type), return_stmt->expr,
         strdup("return type mismatch"));
  OK;
});

ANALYZER(declaration, {
  NAME_OF(declaration);
  bool is_static = IS_GLOBAL || declaration->is_constant;

  SymbolData *symbol_data = NULL;
  EXPECT(add_symbol(allocator, table, name, &symbol_data), node,
         strdup("duplicate declaration"));
  symbol_data->is_static = is_static;
  declaration->value_ptr = &symbol_data->value;

  Result result = ANALYZE_NODE(declaration->expr, value, &symbol_data->type);
  if (result != rDONE) {
    return result;
  }
  if (is_static) {
    symbol_data->value = evaluate_expr(declaration->expr);
  }
  OK;
});

ANALYZER(module, {
  ANALYZE_ARRAY(module->declarations, &module->table);
  OK;
});

ANALYZER_SIGNATURE(node) {
  if (node->stage >= sANALYZED)
    return rDONE;
  SWITCH(
      node,
      panicf("analyze not implemented for node: %s", ast_node_name(node->kind)),
      {
        ACASE(name);
        ACASE(integer);
        ACASE(string);
        ACASE(unary);
        ACASE(binary);
        ACASE(function_call);
        ACASE(function);
        ACASE(param);
        ACASE(for_loop);
        ACASE(assign);
        ACASE(return_stmt);
        ACASE(declaration);
        ACASE(module);
      });
}

void print_analyze_errors(const char *source, TypeErrorArray type_errors) {
  for (size_t i = 0; i < type_errors.length; i++) {
    TypeError error = type_errors.data[i];
    eprintf("Error: %s\n", error.message);

    OffsetInfo oi = get_offset_info(source, error.span.offset);
    eprintf(">>> line %zu, column %zu\n", oi.line_number, oi.column_number);
    eprintf(" | %.*s\n", (int)oi.line_length, oi.line);
    eprintf(" | %*s", (int)oi.column_number, " ");
    for (size_t i = 0;
         i < MIN(error.span.length, oi.line_length - oi.column_number); i++)
      eprintf("~");
    eprintf("\n");
  }
}

void free_analyze_errors(TypeErrorArray *type_errors) {
  for (size_t i = 0; i < type_errors->length; i++) {
    TypeError error = type_errors->data[i];
    free(error.message);
  }
  free(type_errors->data);
  type_errors->data = NULL;
}

bool analyze(AST *ast, TypeErrorArray *error_data) {
  Allocator allocator = {0};
  Table table = {0};
  Result result = 0;
  for (size_t i = 0;; i++) {
    eprintf("INFO: analyze iteration: %zu\n", i);
    result = analyze_node(&allocator, ast->head, &table, (Type){0}, NULL,
                          error_data, true);
    if (result != rBLOCKED)
      break;
  }
  // NOTE: this leaves modified values in the AST.
  // Should these be zeroed after analysis or should the allocator be passed
  // along? Or should they simply be in a strange state?
  ra_free_all(&allocator);
  return result == rDONE;
}
