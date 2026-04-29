
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
                           error_data, is_static, flags);                      \
  })

// analyze a node, only returning on error
#define ANALYZE_STMT($node, $name)                                             \
  Type $name##_type = {0};                                                     \
  {                                                                            \
    Result result = analyze_node(allocator, $node, table, return_type,         \
                                 &$name##_type, error_data, is_static, flags); \
    if (result == rERROR)                                                      \
      FAIL;                                                                    \
    blocked |= result == rBLOCKED;                                             \
  }

// analyze a node, returning on error or UNKNOWN type
#define ANALYZE($node, $name)                                                  \
  ANALYZE_STMT($node, $name);                                                  \
  if (blocked)                                                                 \
    BLOCK;

#define ANALYZE_TYPE($node, $name)                                             \
  ANALYZE($node, $name);                                                       \
  EXPECT(($name##_type.kind == tyTYPE), $node,                                 \
         ssprintf("not a type: %s", type_name($name##_type.kind)));

#define ANALYZE_ARRAY($array, $table)                                          \
  ITER_ARRAY($array, element, {                                                \
    Table *table = $table;                                                     \
    ANALYZE_STMT(element, element);                                            \
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
  rERROR,
  rBLOCKED,
  rOK,
} Result;

typedef enum {
  ANYTHING_CHANGED = 1 << 0,
  BLOCKED = 1 << 1,
  ERROR_ON_BLOCK = 1 << 2,
} Flags;

#define OK                                                                     \
  {                                                                            \
    if (blocked)                                                               \
      BLOCK;                                                                   \
    node->stage = sANALYZED;                                                   \
    *flags |= ANYTHING_CHANGED;                                                \
    return rOK;                                                                \
  }
#define FAIL return rERROR
#define BLOCK return rBLOCKED

#define IS_GLOBAL (table->parent == NULL)

#define ANALYZER_SIGNATURE($name)                                              \
  Result analyze_##$name(Allocator *allocator, Node *node, Table *table,       \
                         Type return_type, Type *out_type,                     \
                         TypeErrorArray *error_data, bool is_static,           \
                         Flags *flags)

#define ANALYZER($name, ...)                                                   \
  ANALYZER_SIGNATURE($name) {                                                  \
    __auto_type $name = &node->$name;                                          \
    bool blocked = false;                                                      \
    __VA_ARGS__;                                                               \
    UNUSED(out_type);                                                          \
    UNUSED(table);                                                             \
    UNUSED(return_type);                                                       \
    UNUSED(error_data);                                                        \
    UNUSED(is_static);                                                         \
    UNUSED(allocator);                                                         \
    UNUSED(blocked);                                                           \
  }

ANALYZER_SIGNATURE(node);

ANALYZER(name, {
  SymbolData *symbol_data = NULL;
  if (!get_symbol(table, name->name, &symbol_data)) {
    eprintf("INFO: compilation blocked by undefined symbol `%.*s` \n",
            (int)name->name.length, name->name.text);
    EXPECT(((*flags & ERROR_ON_BLOCK) == 0), node, strdup("undefined symbol"));
    BLOCK;
  }
  EXPECT((!is_static || symbol_data->is_static), node,
         strdup("static declaration refers to runtime variable"));
  name->value_ptr = &symbol_data->value;
  if (symbol_data->type.kind == tyUNKNOWN)
    BLOCK;
  *out_type = symbol_data->type;
  out_type->is_bound = true;
  OK;
});

ANALYZER(integer, {
  EXPECT((sscanf(integer->text, "%zd", &integer->value) == 1), node,
         strdup("integer is too large"));
  *out_type = PRIM_TYPE(tyINT);
  OK;
});

ANALYZER(string, {
  UNUSED(string);
  String value = {.text = ra_calloc(allocator, string->length)};
  for (size_t i = 0; i < string->length; i++) {
    char c = string->text[i];
    if (c == '\\') {
      i++;
      c = string->text[i];
      if (c == 'n')
        c = '\n';
      else if (c == 'r')
        c = '\r';
      else if (c == 't')
        c = '\t';
      else if (c == 'f')
        c = '\f';
      else if (c == '\\')
        c = '\\';
      else if (c == '"')
        c = '"';
      else
        EXPECT(false, node, ssprintf("invalid escape sequence: `\\%c`", c));
    }
    value.text[value.length] = c;
    value.length++;
  }
  string->value = value;
  *out_type = PRIM_TYPE(tySTRING);
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
           strdup("types on both sides of an arithmetic operator "
                  "must be the same"));
    *out_type = left_type;
    OK;
  }
  if (IS_ONE_OF(binary->op, tLT, tGT, tLT_EQ, tGT_EQ)) {
    EXPECT((left_type.kind == tyINT), binary->left,
           ssprintf("cannot apply operator %s to non-numeric type",
                    token_name(binary->op)));
    EXPECT((right_type.kind == tyINT), binary->right,
           ssprintf("cannot apply operator %s to non-numeric type",
                    token_name(binary->op)));
    EXPECT((left_type.kind == right_type.kind), node,
           strdup("types on both sides of a comparison operator "
                  "must be the same"));
    *out_type = PRIM_TYPE(tyBOOL);
    OK;
  }
  if (binary->op == tEQ_EQ) {
    EXPECT((left_type.kind == tyINT), binary->left,
           ssprintf("cannot apply operator %s to non-numeric type (currently)",
                    token_name(binary->op)));
    EXPECT((right_type.kind == tyINT), binary->right,
           ssprintf("cannot apply operator %s to non-numeric type (currently)",
                    token_name(binary->op)));
    EXPECT((left_type.kind == right_type.kind), node,
           strdup("types on both sides of a comparison operator "
                  "must be the same"));
    *out_type = PRIM_TYPE(tyBOOL);
    OK;
  }
  panicf("Unknown binary operator: `%s`", token_display_name(binary->op));
});

ANALYZER(function_call, {
  EXPECT((!is_static), node, strdup("cannot call function in static code"));
  ANALYZE(function_call->function, function);
  EXPECT((function_type.kind == tyFUNCTION), function_call->function,
         strdup("not a function"));

  __auto_type function = function_type.function;
  if (function._return != NULL) {
    *out_type = *function._return;
  }
  TypeArray param_types = function.params;

  size_t expected_length = param_types.length;
  size_t found_length = function_call->args.length;
  EXPECT((expected_length == found_length), node,
         ssprintf("argument count mismatch: expected %zu, but found %zu",
                  expected_length, found_length));

  ITER_ARRAY(function_call->args, arg, {
    ANALYZE(arg, arg);
    Type param_type = param_types.data[i];
    EXPECT((type_eq(arg_type, param_type)), arg,
           ssprintf("argument type mismatch: expected %s, but found %s",
                    FMT_TYPE(param_type), FMT_TYPE(arg_type)));
  });
  OK;
});

// FIXME: in order to make this whole analyze function resumable, I
// need to store the types of parameters and such that have been analyzed
// maybe just store them in the AST? typed AST information can also be
// useful for other things
ANALYZER(function, {
  EXPECT(IS_GLOBAL, node,
         strdup("functions can only be defined in the global scope"));

  if (node->stage < sTYPED) {
    function->table = (Table){.parent = table, .data = NULL, .length = 0};
    Table *table = &function->table;

    if (out_type->kind != tyFUNCTION) {
      *out_type = (Type){.kind = tyFUNCTION, .is_constant = true};
      out_type->function.params = (TypeArray){
          .data = ra_calloc(allocator, sizeof(Type) * function->params.length),
          .length = function->params.length};
    }
    ITER_ARRAY(function->params, param_node, {
      ANALYZE_STMT(param_node, param);
      out_type->function.params.data[i] = param_type;
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
  NAME_OF(param);
  if (param->symbol_data == NULL) {
    EXPECT(
        add_symbol(allocator, table, name, &param->symbol_data), node,
        ssprintf("duplicate parameter: `%.*s`", (int)name.length, name.text));
  }
  ANALYZE_TYPE(param->type, param);
  Type type = evaluate_expr(param->type).type;
  param->symbol_data->type = type;
  *out_type = type;
  OK;
});

ANALYZER(for_loop, {
  ANALYZE_STMT(for_loop->init, init);
  ANALYZE(for_loop->cond, cond);
  ANALYZE_STMT(for_loop->step, step);
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
  EXPECT(target_type.is_bound, node,
         strdup("cannot assign to temporary value"));
  EXPECT(target_type.is_constant, node, strdup("cannot assign to constant"));
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

  if (declaration->symbol_data == NULL) {
    EXPECT(add_symbol(allocator, table, name, &declaration->symbol_data), node,
           strdup("duplicate declaration"));
    declaration->symbol_data->is_static = is_static;
  }
  ANALYZE(declaration->expr, expr);
  declaration->symbol_data->type = expr_type;
  if (is_static) {
    declaration->symbol_data->value = evaluate_expr(declaration->expr);
  }
  OK;
});

ANALYZER(module, {
  ANALYZE_ARRAY(module->declarations, &module->table);
  OK;
});

ANALYZER_SIGNATURE(node) {
  if (node->stage >= sANALYZED)
    return rOK;
  SWITCH(node, {
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
  default:
    panicf("analyze not implemented for node: %s", ast_node_name(node->kind));
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
  Table table = {0};
  Flags flags = 0;
  Result result = rBLOCKED;
  for (size_t i = 0;; i++) {
    eprintf("INFO: analysis iteration: %zu\n", i);
    // TODO: make sure the symbol tables (also allocated using random_allocator)
    // are freed, but the symbol data is not
    result = analyze_node(&ast->random_allocator, ast->head, &table, (Type){0},
                          NULL, error_data, true, &flags);
    if (result != rBLOCKED)
      break;
    flags = (flags & ANYTHING_CHANGED) != 0 ? ERROR_ON_BLOCK : 0;
  }
  return result;
}
