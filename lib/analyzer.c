
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

#define NAME_OF($decl)                                                         \
  assert(($decl)->name->kind == aNAME);                                        \
  Name name = ($decl)->name->name.name;

#define ACASE($name)                                                           \
  CASE($name, {                                                                \
    return analyze_##$name(allocator, node, table, struct_list, return_type,   \
                           out_type, error_data, is_static, flags);            \
  })

// Analyze a node, only returning on error, and write the type to $out_type.
#define TRY_ANALYZE_TO_TYPE($node, $out_type)                                  \
  {                                                                            \
    Result result =                                                            \
        analyze_node(allocator, $node, table, struct_list, return_type,        \
                     $out_type, error_data, is_static, flags);                 \
    if (result == rERROR)                                                      \
      FAIL;                                                                    \
    blocked |= result == rBLOCKED;                                             \
  }

// Analyze a node, only returning on error.
#define TRY_ANALYZE($node, $name)                                              \
  Type $name##_type = {0};                                                     \
  TRY_ANALYZE_TO_TYPE($node, &$name##_type)

// Analyze a node, returning on error or block, and write the type to $out_type
#define ANALYZE_TO_TYPE($node, $out_type)                                      \
  TRY_ANALYZE_TO_TYPE($node, $out_type);                                       \
  if (blocked)                                                                 \
    BLOCK;

// Analyze a node, returning on error or block.
#define ANALYZE($node, $name)                                                  \
  TRY_ANALYZE($node, $name);                                                   \
  if (blocked)                                                                 \
    BLOCK;

#define ANALYZE_TYPE($node, $name)                                             \
  ANALYZE($node, $name##_type);                                                \
  EXPECT(($name##_type_type.kind == tyTYPE), $node,                            \
         ssprintf("not a type: %s", type_name($name##_type_type.kind)));       \
  Type $name##_type = evaluate_expr($node).type;

#define ANALYZE_ARRAY($array)                                                  \
  {                                                                            \
    ITER_ARRAY($array, element, { TRY_ANALYZE(element, element); });           \
    if (blocked)                                                               \
      BLOCK;                                                                   \
  }

void add_error(AnalyzeErrorArray *error_data, Span span, char *message) {
  AnalyzeError error = {.span = span, .message = message};
  error_data->data = realloc(error_data->data,
                             sizeof(AnalyzeError) * (error_data->length + 1));
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
    node->is_analyzed = true;                                                  \
    *flags |= ANYTHING_CHANGED;                                                \
    return rOK;                                                                \
  }
#define FAIL return rERROR
#define BLOCK return rBLOCKED

#define IS_GLOBAL (table->parent == NULL)

#define ANALYZER_SIGNATURE($name)                                              \
  Result analyze_##$name(Allocator *allocator, Node *node, Table *table,       \
                         StructList *struct_list, Type return_type,            \
                         Type *out_type, AnalyzeErrorArray *error_data,        \
                         bool is_static, Flags *flags)

#define ANALYZER($name, ...)                                                   \
  ANALYZER_SIGNATURE($name) {                                                  \
    __auto_type $name = &node->$name;                                          \
    bool blocked = false;                                                      \
    __VA_ARGS__;                                                               \
    UNUSED(out_type);                                                          \
    UNUSED(table);                                                             \
    UNUSED(struct_list);                                                       \
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

ANALYZER(boolean, {
  if (boolean->length == 4)
    boolean->value = true;
  else
    boolean->value = false;
  *out_type = PRIM_TYPE(tyBOOL);
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

#define ANALYZE_BINARY($expected_type, $type_error, $equal_error, $out_type)   \
  EXPECT((left_type.kind == $expected_type), binary->left, $type_error);       \
  EXPECT((right_type.kind == $expected_type), binary->right, $type_error);     \
  EXPECT((left_type.kind == right_type.kind), node, $equal_error);             \
  *out_type = $out_type;                                                       \
  OK;

ANALYZER(binary, {
  ANALYZE(binary->left, left);
  ANALYZE(binary->right, right);
  if (IS_ONE_OF(binary->op, tPLUS, tMINUS, tSTAR, tSLASH)) {
    ANALYZE_BINARY(
        tyINT, strdup("cannot apply arithmetic operator to non-numeric type"),
        strdup(
            "types on both sides of an arithmetic operator must be the same"),
        left_type);
  }
  if (IS_ONE_OF(binary->op, tLT, tGT, tLT_EQ, tGT_EQ)) {
    ANALYZE_BINARY(
        tyINT,
        ssprintf("cannot apply operator %s to non-numeric type",
                 token_name(binary->op)),
        strdup("types on both sides of a comparison operator must be the same"),
        PRIM_TYPE(tyBOOL));
  }
  if (binary->op == tEQ_EQ) {
    ANALYZE_BINARY(
        tyINT,
        ssprintf("cannot apply operator == to non-numeric type (currently)"),
        strdup("types on both sides of a comparison operator must be the same"),
        PRIM_TYPE(tyBOOL));
  }
  if (binary->op == tAMP_AMP) {
    ANALYZE_BINARY(tyBOOL,
                   strdup("cannot apply operator && to non-boolean type"), "",
                   PRIM_TYPE(tyBOOL));
  }
  if (binary->op == tBAR_BAR) {
    ANALYZE_BINARY(tyBOOL,
                   strdup("cannot apply operator || to non-boolean type"), "",
                   PRIM_TYPE(tyBOOL));
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

ANALYZER(param, {
  NAME_OF(param);
  if (param->symbol_data == NULL) {
    EXPECT(add_symbol(allocator, table, name, &param->symbol_data), node,
           strdup("duplicate parameter name"));
  }
  ANALYZE_TYPE(param->type, param);
  param->symbol_data->type = param_type;
  *out_type = param_type;
  OK;
});

ANALYZER(function, {
  EXPECT(IS_GLOBAL, node,
         strdup("functions can only be defined in the global scope"));
  EXPECT(is_static, node, strdup("functions can only be defined as constants"));

  function->table.parent = table;
  Table *table = &function->table;

  if (out_type->kind == tyUNKNOWN) {
    out_type->is_constant = true;
    if (out_type->function.params.data == NULL) {
      out_type->function.params = (TypeArray){
          .data = ra_calloc(allocator, sizeof(Type) * function->params.length),
          .length = function->params.length};
    }
    ITER_ARRAY(function->params, param_node, {
      ANALYZE(param_node, param);
      out_type->function.params.data[i] = param_type;
    });
    out_type->function._return = ra_calloc(allocator, sizeof(Type));
    if (function->return_type != NULL) {
      ANALYZE_TYPE(function->return_type, return);
      *out_type->function._return = return_type;
    } else {
      *out_type->function._return = PRIM_TYPE(tyVOID);
    }
    // Set the type to tyFUNCTION from tyUNKNOWN,
    // indicating that the type has been determined.
    out_type->kind = tyFUNCTION;
  }
  bool is_static = false;
  Type return_type = *out_type->function._return;
  ANALYZE_ARRAY(function->stmts);
  OK;
});

ANALYZER(if_stmt, {
  ANALYZE(if_stmt->cond, cond);
  EXPECT((cond_type.kind == tyBOOL), node,
         strdup("expected boolean type as if-statement condition"));

  if_stmt->table.parent = table;
  Table *table = &if_stmt->table;
  ANALYZE_ARRAY(if_stmt->then_stmts);
  ANALYZE_ARRAY(if_stmt->else_stmts);  
  OK;
});

ANALYZER(while_loop, {
  ANALYZE(while_loop->cond, cond);
  EXPECT((cond_type.kind == tyBOOL), node,
         strdup("expected boolean type as while-loop condition"));

  while_loop->table.parent = table;
  Table *table = &while_loop->table;
  ANALYZE_ARRAY(while_loop->stmts);
  OK;
});

ANALYZER(for_loop, {
  for_loop->table.parent = table;
  Table *table = &for_loop->table;

  TRY_ANALYZE(for_loop->init, init);
  ANALYZE(for_loop->cond, cond);
  TRY_ANALYZE(for_loop->step, step);
  EXPECT((cond_type.kind == tyBOOL), node,
         strdup("expected boolean type as for-loop condition"));
  ANALYZE_ARRAY(for_loop->stmts);
  OK;
});

ANALYZER(assign, {
  ANALYZE(assign->target, target);
  ANALYZE(assign->expr, expr);

  if (assign->op == tEQ) {
  } else if (IS_ONE_OF(assign->op, tPLUS_EQ, tMINUS_EQ, tSTAR_EQ, tSLASH_EQ)) {
    EXPECT(
        (target_type.kind == tyINT), assign->target,
        strdup(
            "cannot apply arithmetic assignment operator to non-numeric type"));
    EXPECT(
        (expr_type.kind == tyINT), assign->expr,
        strdup(
            "cannot apply arithmetic assignment operator to non-numeric type"));
  } else {
    panicf("Unexpected assignment operator: %s", token_name(assign->op));
  }
  EXPECT(type_eq(target_type, expr_type), node,
         strdup("assigment type mismatch"));
  EXPECT(target_type.is_bound, node,
         strdup("cannot assign to temporary value"));
  EXPECT(target_type.is_constant, node, strdup("cannot assign to constant"));
  OK;
});

ANALYZER(return_stmt, {
  ANALYZE(return_stmt->expr, expr);
  EXPECT(type_eq(expr_type, return_type), return_stmt->expr,
         ssprintf("return type mismatch: expected %s, but found %s",
                  type_name(return_type.kind), type_name(expr_type.kind)));
  OK;
});

ANALYZER(field, {
  NAME_OF(field);
  if (field->symbol_data == NULL) {
    EXPECT(add_symbol(allocator, table, name, &field->symbol_data), node,
           strdup("duplicate field name"));
  }
  ANALYZE_TYPE(field->type, field);
  field->symbol_data->type = field_type;
  *out_type = field_type;
  OK;
});

ANALYZER(_struct, {
  EXPECT(IS_GLOBAL, node,
         strdup("structs can only be defined in the global scope"));
  EXPECT(is_static, node, strdup("structs can only be defined as constants"));

  _struct->id = add_struct(allocator, struct_list);
  *out_type =
      (Type){.kind = tySTRUCT, .is_constant = true, ._struct = _struct->id};
  StructInfo *info = &struct_list->data[_struct->id];

  if (info->fields.data == NULL) {
    info->fields = (FieldInfoArray){
        .data =
            ra_calloc(allocator, sizeof(FieldInfo) * _struct->fields.length),
        .length = _struct->fields.length};
  }
  ITER_ARRAY(_struct->fields, field_node, {
    ANALYZE(field_node, field);
    info->fields.data[i] = (FieldInfo){
        .name = field_node->field.name->name.name, .type = field_type};
  });
  OK;
});

static Type VOID_TYPE = PRIM_TYPE(tyVOID);
static Type MAIN_FUNCTION_TYPE = {.kind = tyFUNCTION,
                                  .is_constant = true,
                                  .function = {._return = &VOID_TYPE}};

ANALYZER(decl, {
  NAME_OF(decl);
  bool is_static = IS_GLOBAL || decl->is_constant;

  if (decl->symbol_data == NULL) {
    EXPECT(add_symbol(allocator, table, name, &decl->symbol_data), decl->name,
           strdup("duplicate declaration"));
    decl->symbol_data->is_static = is_static;
  }
  assert(decl->symbol_data != NULL);
  Type *expr_type = &decl->symbol_data->type;
  ANALYZE_TO_TYPE(decl->expr, expr_type);
  if (name_eq_string(decl->name->name.name, "main")) {
    EXPECT(type_eq(*expr_type, MAIN_FUNCTION_TYPE), decl->name,
           ssprintf("invalid `main` function type, expected `() -> void`"));
  }
  if (is_static) {
    decl->symbol_data->value = evaluate_expr(decl->expr);
  }
  OK;
});

ANALYZER(module, {
  Table *table = &module->table;
  ANALYZE_ARRAY(module->decls);
  OK;
});

ANALYZER_SIGNATURE(node) {
  if (node->is_analyzed)
    return rOK;
  SWITCH(node, {
    ACASE(name);
    ACASE(integer);
    ACASE(boolean);
    ACASE(string);
    ACASE(unary);
    ACASE(binary);
    ACASE(function_call);
    ACASE(function);
    ACASE(param);
    ACASE(if_stmt);
    ACASE(while_loop);
    ACASE(for_loop);
    ACASE(assign);
    ACASE(return_stmt);
    ACASE(field);
    ACASE(_struct);
    ACASE(decl);
    ACASE(module);
  });
  panicf("analyze not implemented for node: %s", ast_node_name(node->kind));
}

void print_analyze_errors(const char *source, AnalyzeErrorArray type_errors) {
  for (size_t i = 0; i < type_errors.length; i++) {
    AnalyzeError error = type_errors.data[i];
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

void free_analyze_errors(AnalyzeErrorArray *type_errors) {
  for (size_t i = 0; i < type_errors->length; i++) {
    AnalyzeError error = type_errors->data[i];
    free(error.message);
  }
  free(type_errors->data);
  type_errors->data = NULL;
}

bool analyze(AST *ast, AnalyzeErrorArray *error_data) {
  Table table = {0};
  StructList struct_list = {0};
  Flags flags = 0;
  Result result = rBLOCKED;
  for (size_t i = 0;; i++) {
    eprintf("INFO: analysis iteration: %zu\n", i);
    // TODO: make sure the symbol tables (also allocated using random_allocator)
    // are freed, but the symbol data is not
    result =
        analyze_node(&ast->random_allocator, ast->head, &table, &struct_list,
                     (Type){0}, NULL, error_data, true, &flags);
    if (result != rBLOCKED)
      break;
    flags = (flags & ANYTHING_CHANGED) != 0 ? ERROR_ON_BLOCK : 0;
  }
  return result;
}
