
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

typedef enum {
  ANYTHING_CHANGED = 1 << 0,
  BLOCKED = 1 << 1,
  ERROR_ON_BLOCK = 1 << 2,
} Flags;

typedef struct {
  Allocator *allocator;
  AnalyzeErrorArray *error_data;
  bool anything_changed;
  bool error_on_block;
} State;

#define ALLOC($size) ra_calloc(state->allocator, $size)

#define NAME_OF($decl)                                                         \
  assert(($decl)->name->kind == aNAME);                                        \
  Name name = ($decl)->name->name.name;

#define ACASE($name)                                                           \
  CASE($name, {                                                                \
    return analyze_##$name(state, node, table, return_type, frame_length,      \
                           is_static);                                         \
  })

// Analyze a node, only returning on error.
#define TRY_ANALYZE($node, $name)                                              \
  {                                                                            \
    Result result = analyze_node(state, $node, table, return_type,             \
                                 frame_length, is_static);                     \
    if (result == rERROR)                                                      \
      FAIL;                                                                    \
    blocked |= result == rBLOCKED;                                             \
  }                                                                            \
  Type $name##_type = ($node)->type;                                           \
  UNUSED($name##_type)

// Analyze a node, returning on error or block.
#define ANALYZE($node, $name)                                                  \
  TRY_ANALYZE($node, $name);                                                   \
  if (blocked)                                                                 \
    BLOCK;

#define ANALYZE_TYPE($node, $name)                                             \
  ANALYZE($node, $name##_type);                                                \
  EXPECT(($name##_type_type.kind == tyTYPE), $node,                            \
         ssprintf("not a type: %s", type_name($name##_type_type.kind)));       \
  Type $name##_type = evaluate_expr($node, 0, NULL).type;

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
    add_error(state->error_data, node->span, $message);                        \
    return rERROR;                                                             \
  }

typedef enum {
  rERROR,
  rBLOCKED,
  rOK,
} Result;

#define OK                                                                     \
  {                                                                            \
    if (blocked)                                                               \
      BLOCK;                                                                   \
    node->is_analyzed = true;                                                  \
    state->anything_changed = true;                                            \
    return rOK;                                                                \
  }
#define FAIL return rERROR
#define BLOCK return rBLOCKED

#define IS_GLOBAL (table->parent == NULL)

#define ANALYZER_SIGNATURE($name)                                              \
  Result analyze_##$name(State *state, Node *node, Table *table,               \
                         Type return_type, size_t *frame_length,               \
                         bool is_static)

#define ANALYZER($name, ...)                                                   \
  ANALYZER_SIGNATURE($name) {                                                  \
    __auto_type $name = &node->$name;                                          \
    bool blocked = false;                                                      \
    __VA_ARGS__;                                                               \
    UNUSED(table);                                                             \
    UNUSED(state);                                                             \
    UNUSED(return_type);                                                       \
    UNUSED(is_static);                                                         \
    UNUSED(blocked);                                                           \
    UNUSED(frame_length);                                                      \
  }

ANALYZER_SIGNATURE(node);

ANALYZER(name, {
  Symbol symbol = {0};
  if (!get_symbol(table, name->name, &symbol)) {
    eprintf("INFO: compilation blocked by undefined symbol `%.*s` \n",
            (int)name->name.length, name->name.text);
    EXPECT(!state->error_on_block, node, strdup("undefined symbol"));
    BLOCK;
  }
  EXPECT((!is_static || symbol.is_static), node,
         strdup("static declaration refers to runtime variable"));
  name->is_static = symbol.is_static;
  if (symbol.is_static) {
    assert(symbol.node->kind == aDECL);
    name->static_value_ptr = &symbol.node->decl.static_value;
  } else {
    name->local_index = symbol.stack_offset; // FIXME: fix local indices
  }
  if (symbol.node->type.kind == tyUNKNOWN)
    BLOCK;
  node->type = symbol.node->type;
  node->type.is_bound = true;
  OK;
});

ANALYZER(integer, {
  EXPECT((sscanf(integer->text, "%zd", &integer->value) == 1), node,
         strdup("integer is too large"));
  node->type = PRIM_TYPE(tyINT);
  OK;
});

ANALYZER(boolean, {
  if (boolean->length == 4)
    boolean->value = true;
  else
    boolean->value = false;
  node->type = PRIM_TYPE(tyBOOL);
  OK;
});

ANALYZER(string, {
  UNUSED(string);
  String value = {.text = ALLOC(string->length)};
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
  node->type = PRIM_TYPE(tySTRING);
  OK;
});

ANALYZER(unary, {
  ANALYZE(unary->expr, expr);
  if (unary->op == tMINUS) {
    EXPECT((expr_type.kind != tyINT), unary->expr,
           strdup("cannot apply unary operator `-` to non-numeric type"));
    node->type = expr_type;
    OK;
  }
  panicf("Unknown unary operator: `%s`", token_display_name(unary->op));
});

#define ANALYZE_BINARY($expected_type, $type_error, $equal_error, $out_type)   \
  EXPECT((left_type.kind == $expected_type), binary->left, $type_error);       \
  EXPECT((right_type.kind == $expected_type), binary->right, $type_error);     \
  EXPECT((left_type.kind == right_type.kind), node, $equal_error);             \
  node->type = $out_type;                                                      \
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
    node->type = *function._return;
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

ANALYZER(field_inst, {
  ANALYZE(field_inst->expr, expr);
  node->type = expr_type;
  OK;
});

Type get_type_value(ASTNode *type_node) {
  assert(type_node->kind == aNAME);
  assert(type_node->type.kind != tyTYPE);
  assert(type_node->name.static_value_ptr != NULL);
  return type_node->name.static_value_ptr->type;
}

ANALYZER(struct_inst, {
  ANALYZE_TYPE(struct_inst->type, struct);
  assert(struct_type.kind != tyUNKNOWN);
  EXPECT(struct_type.kind == tySTRUCT, struct_inst->type,
         ssprintf("expected struct type, but found %s",
                  type_name(struct_type.kind)));

  ASTNode *_struct_node = struct_type._struct;
  __auto_type _struct = &_struct_node->_struct;

  bool used[_struct->fields.length];
  memset(used, 0, sizeof(used));

  struct_inst->local_index = *frame_length;
  *frame_length += _struct->flat_length;

  ITER_ARRAY(struct_inst->fields, field_inst_node, {
    __auto_type field_inst = &field_inst_node->field_inst;
    Type expected_type = {0};

    ITER_ARRAY(_struct->fields, field_node, {
      __auto_type field = &field_node->field;
      if (name_eq(field->name->name.name, field_inst->name->name.name)) {
        EXPECT(!used[i], field_inst_node, strdup("duplicate field"));
        used[i] = true;
        expected_type = get_type_value(field->type);
        break;
      }
    });
    // NOTE: not sure if tyUNKNOWN can be the field type normally as well
    EXPECT(expected_type.kind != tyUNKNOWN, field_inst->name,
           strdup("unknown field"));
    ANALYZE(field_inst_node, found);
    EXPECT(type_eq(expected_type, found_type), field_inst->expr,
           ssprintf("expression type mismatch: expected %s, but found %s",
                    FMT_TYPE(expected_type), FMT_TYPE(found_type)));
  });
  node->type = struct_type;
  OK;
});

ANALYZER(param, {
  NAME_OF(param);
  if (!param->symbol_added) {
    EXPECT(
        add_symbol(state->allocator, table, name, node, false, *frame_length),
        node, strdup("duplicate parameter name"));
    *frame_length += 1;
    param->symbol_added = true;
  }
  ANALYZE_TYPE(param->type, param);
  node->type = param_type;
  OK;
});

ANALYZER(function, {
  EXPECT(IS_GLOBAL, node,
         strdup("functions can only be defined in the global scope"));
  EXPECT(is_static, node, strdup("functions can only be defined as constants"));

  function->table.parent = table;
  Table *table = &function->table;
  Type *type = &node->type;
  size_t *frame_length = &function->frame_length;

  if (type->kind == tyUNKNOWN) {
    type->is_constant = true;
    if (type->function.params.data == NULL) {
      type->function.params =
          (TypeArray){.data = ALLOC(sizeof(Type) * function->params.length),
                      .length = function->params.length};
    }
    ITER_ARRAY(function->params, param_node, {
      ANALYZE(param_node, param);
      type->function.params.data[i] = param_type;
    });
    type->function._return = ALLOC(sizeof(Type));
    if (function->return_type != NULL) {
      ANALYZE_TYPE(function->return_type, return);
      *type->function._return = return_type;
    } else {
      *type->function._return = PRIM_TYPE(tyVOID);
    }
    // Set the type to tyFUNCTION from tyUNKNOWN,
    // indicating that the type has been determined.
    type->kind = tyFUNCTION;
  }
  bool is_static = false;
  Type return_type = *type->function._return;
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
  ANALYZE_TYPE(field->type, field);
  node->type = field_type;
  OK;
});

size_t flat_length(Type type) {
  assert(type.kind != tyUNKNOWN);
  if (type.kind == tySTRUCT) {
    return type._struct->_struct.flat_length;
  } else if (type.kind == tyVOID) {
    return 0;
  } else {
    return 1;
  }
}

ANALYZER(_struct, {
  EXPECT(IS_GLOBAL, node,
         strdup("structs can only be defined in the global scope"));
  EXPECT(is_static, node, strdup("structs can only be defined as constants"));

  node->type = (Type){.kind = tySTRUCT, .is_constant = true, ._struct = node};
  _struct->flat_length = 0;

  ITER_ARRAY(_struct->fields, field_node, {
    ANALYZE(field_node, field);
    _struct->flat_length += flat_length(field_type);
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

  if (!decl->symbol_added) {
    size_t local_index = *frame_length;
    EXPECT(
        add_symbol(state->allocator, table, name, node, is_static, local_index),
        decl->name, strdup("duplicate declaration"));
    *frame_length += 1;
    decl->symbol_added = true;
    decl->local_index = local_index;
  }
  TRY_ANALYZE(decl->expr, expr);
  // function type can be determined despite blocking
  node->type = expr_type;
  if (blocked)
    BLOCK;

  if (name_eq_string(decl->name->name.name, "main")) {
    EXPECT(type_eq(expr_type, MAIN_FUNCTION_TYPE), decl->name,
           ssprintf("invalid `main` function type, expected `() -> void`"));
  }
  if (is_static) {
    decl->static_value = evaluate_expr(decl->expr, 0, NULL);
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
    ACASE(field_inst);
    ACASE(struct_inst);
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
  size_t global_count = 0;
  State state = {.allocator = &ast->random_allocator, .error_data = error_data};
  Result result = rBLOCKED;
  for (size_t i = 0;; i++) {
    eprintf("INFO: analysis iteration: %zu\n", i);
    // TODO: make sure the symbol tables (also allocated using random_allocator)
    // are freed, but the static values are not
    result =
        analyze_node(&state, ast->head, &table, (Type){0}, &global_count, true);
    if (result != rBLOCKED)
      break;

    state.error_on_block = state.anything_changed;
    state.anything_changed = false;
  }
  return result;
}
