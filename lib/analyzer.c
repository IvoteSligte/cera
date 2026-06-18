
#include "analyzer.h"
#include "analyzer_shared.h"
#include "ast.h"
#include "ast_macro.h"
#include "offset.h"

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

static Type VOID_TYPE = PRIM_TYPE(tyVOID);
static Type MAIN_FUNCTION_TYPE = {.kind = tyFUNCTION,
                                  .is_constant = true,
                                  .function = {._return = &VOID_TYPE}};

#define ALLOC($size) ra_calloc(state->allocator, $size)

#define ACASE($name)                                                           \
  CASE($name, {                                                                \
    return analyze_##$name(state, node, table, return_type, is_static,         \
                           in_loop);                                           \
  })

// Analyze a node, only returning on error.
#define TRY_ANALYZE($node, $name)                                              \
  {                                                                            \
    Result result =                                                            \
        analyze_node(state, $node, table, return_type, is_static, in_loop);    \
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
  Type $name##_type = {0};                                                     \
  {                                                                            \
    ANALYZE($node, $name##_type);                                              \
    EXPECT(($name##_type_type.kind == tyTYPE), $node,                          \
           ssprintf("not a type: %s", type_name($name##_type_type.kind)));     \
    $name##_type = GET_TYPE_VALUE($node);                                      \
    assert($name##_type.kind != tyUNKNOWN);                                    \
  }

#define ANALYZE_ARRAY($array)                                                  \
  {                                                                            \
    ITER_ARRAY($array, element, { TRY_ANALYZE(element, element); });           \
    if (blocked)                                                               \
      BLOCK;                                                                   \
  }

#define DECLARE($node)                                                         \
  if (!$node->symbol_added) {                                                  \
    EXPECT(add_symbol(state->allocator, table, $node->name->name.name, node),  \
           $node->name, strdup("duplicate declaration"));                      \
    $node->symbol_added = true;                                                \
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
                         Type return_type, bool is_static, bool in_loop)

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
    UNUSED(in_loop);                                                           \
  }

ANALYZER_SIGNATURE(node);

ANALYZER(name, {
  if (!get_symbol(table, name->name, &node->type, &name->decl,
                  &name->builtin)) {
    eprintf("INFO: compilation blocked by undefined symbol `%.*s` \n",
            (int)name->name.length, name->name.text);
    EXPECT(!state->error_on_block, node, strdup("undefined symbol"));
    BLOCK;
  }
  /* TODO: EXPECT((!is_static || symbol.value.kind != symDYNAMIC), node, */
  /*        strdup("static declaration refers to runtime variable")); */
  if (node->type.kind == tyUNKNOWN)
    BLOCK;
  /* TODO (only for variables): EXPECT(node->type.kind != tyFUNC_DECL, node,
   * strdup("cannot get address of func_decl")); */
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
  if (unary->op == tBANG) {
    EXPECT((expr_type.kind == tyBOOL), unary->expr,
           ssprintf("cannot apply unary operator `-` to non-boolean type %s",
                    type_name(expr_type.kind)));
    node->type = PRIM_TYPE(tyBOOL);
    OK;
  }
  if (unary->op == tMINUS) {
    EXPECT((expr_type.kind == tyINT), unary->expr,
           ssprintf("cannot apply unary operator `-` to non-numeric type %s",
                    type_name(expr_type.kind)));
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
  if (IS_ONE_OF(binary->op, tEQ_EQ, tBANG_EQ)) {
    ANALYZE_BINARY(
        tyINT,
        ssprintf(
            "cannot apply equality operator to non-numeric type (currently)"),
        strdup("types on both sides of an equality operator must be the same"),
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

ANALYZER(func_call, {
  EXPECT((!is_static), node, strdup("cannot call function in static code"));
  ANALYZE(func_call->function, function);
  EXPECT((function_type.kind == tyFUNCTION), func_call->function,
         strdup("not a function"));

  __auto_type function = function_type.function;
  if (function._return != NULL) {
    node->type = *function._return;
  }
  TypeArray param_types = function.params;

  size_t expected_length = param_types.length;
  size_t found_length = func_call->args.length;
  EXPECT((expected_length == found_length), node,
         ssprintf("argument count mismatch: expected %zu, but found %zu",
                  expected_length, found_length));

  ITER_ARRAY(func_call->args, arg, {
    ANALYZE(arg, arg);
    Type param_type = param_types.data[i];
    EXPECT((type_eq(arg_type, param_type)), arg,
           ssprintf("argument type mismatch: expected %s, but found %s",
                    FMT_TYPE(param_type), FMT_TYPE(arg_type)));
  });
  OK;
});

ANALYZER(ptr_create, {
  ANALYZE(ptr_create->expr, expr);
  EXPECT(expr_type.kind != tyTYPE, node,
         strdup("cannot create pointer to type"));
  EXPECT(ptr_create->expr->kind == aNAME, node,
         strdup("cannot create pointer to temporary value "));
  node->type = (Type){.kind = tyPTR, .pointee_type = ALLOC(sizeof(Type))};
  *node->type.pointee_type = expr_type;
  OK;
});

ANALYZER(ptr_deref, {
  ANALYZE(ptr_deref->expr, expr);
  EXPECT(
      expr_type.kind == tyPTR, node,
      ssprintf("cannot dereference non-pointer type %s", FMT_TYPE(expr_type)))
  node->type = *expr_type.pointee_type;
  OK;
});

ANALYZER(ptr_type, {
  ANALYZE(ptr_type->expr, expr);
  UNUSED(expr_type);
  node->type = PRIM_TYPE(tyTYPE);
  OK;
});

#define GET_TYPE_VALUE($node) get_type_value(state, $node)

// Returns the value of a type expression.
Type get_type_value(State *state, ASTNode *node) {
  if (node->type.kind != tyTYPE) {
    panicf("get_type_value called with non-type %s (expected tyTYPE)\n",
           type_name(node->type.kind));
  }
  // Type expressions can currently only be names and pointers.
  if (node->kind == aPTR_TYPE) {
    Type *expr_type = ALLOC(sizeof(Type));
    *expr_type = GET_TYPE_VALUE(node->ptr_type.expr);
    return (Type){.kind = tyPTR, .pointee_type = expr_type};
  }
  assert(node->kind == aNAME);
  auto name = node->name;

  // builtin type
  Type builtin_type = {0};
  if (get_builtin_type(name.name, &builtin_type)) {
    return builtin_type;
  }
  // user-defined type
  assert(name.decl != NULL);
  SWITCH(name.decl, {
    CASE(struct_decl,
         { return (Type){.kind = tySTRUCT, ._struct = name.decl}; });
  default:
    panicf("not a type (get_type_value): %s", ast_node_name(name.decl->kind));
  });
}

ANALYZER(field_inst, {
  ANALYZE(field_inst->expr, expr);
  node->type = expr_type;
  OK;
});

ANALYZER(struct_inst, {
  ANALYZE_TYPE(struct_inst->type, struct);
  EXPECT(struct_type.kind == tySTRUCT, struct_inst->type,
         ssprintf("expected struct type, but found %s",
                  type_name(struct_type.kind)));
  assert(struct_type._struct->kind == aSTRUCT_DECL);
  __auto_type _struct = &struct_type._struct->struct_decl;

  bool used[MAX(_struct->fields.length, 1)];
  memset(used, 0, sizeof(used));

  ITER_ARRAY(struct_inst->fields, field_inst_node, {
    __auto_type field_inst = &field_inst_node->field_inst;
    Type expected_type = {0};

    ITER_ARRAY(_struct->fields, field_node, {
      __auto_type field = &field_node->field;
      if (name_eq(field->name->name.name, field_inst->name->name.name)) {
        EXPECT(!used[i], field_inst_node, strdup("duplicate field"));
        used[i] = true;
        expected_type = GET_TYPE_VALUE(field->type);
        break;
      }
    });
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

ANALYZER(member, {
  ANALYZE(member->expr, expr);
  EXPECT(expr_type.kind == tySTRUCT, member->expr,
         ssprintf("expected struct, but found %s", type_name(expr_type.kind)));

  ASTNode *struct_decl_node = expr_type._struct;
  assert(struct_decl_node->kind == aSTRUCT_DECL);
  ITER_ARRAY(struct_decl_node->struct_decl.fields, field_node, {
    auto field = &field_node->field;
    if (name_eq(field->name->name.name, member->name->name.name)) {
      node->type = GET_TYPE_VALUE(field->type);
      assert(node->type.kind != tyUNKNOWN);
      member->field_index = i;
      OK;
    }
  });
  EXPECT(false, member->name, strdup("field does not exist"));
});

ANALYZER(param, {
  Name name = param->name->name.name;
  if (!param->symbol_added) {
    EXPECT(add_symbol(state->allocator, table, name, node), node,
           strdup("duplicate parameter name"));
    param->symbol_added = true;
  }
  ANALYZE_TYPE(param->type, param);
  node->type = param_type;
  OK;
});

ANALYZER(func_decl, {
  EXPECT(IS_GLOBAL, node,
         strdup("functions can only be defined in the global scope"));
  EXPECT(is_static, node, strdup("functions can only be defined as constants"));

  DECLARE(func_decl);

  func_decl->table.parent = table;
  Table *table = &func_decl->table;
  Type *type = &node->type;

  if (type->kind == tyUNKNOWN) {
    type->is_constant = true;
    if (type->function.params.data == NULL) {
      type->function.params =
          (TypeArray){.data = ALLOC(sizeof(Type) * func_decl->params.length),
                      .length = func_decl->params.length};
    }
    ITER_ARRAY(func_decl->params, param_node, {
      ANALYZE(param_node, param);
      type->function.params.data[i] = param_type;
    });
    type->function._return = ALLOC(sizeof(Type));
    if (func_decl->return_type != NULL) {
      ANALYZE_TYPE(func_decl->return_type, return);
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
  ANALYZE_ARRAY(func_decl->stmts);

  if (name_eq_string(func_decl->name->name.name, "main")) {
    EXPECT(type_eq(node->type, MAIN_FUNCTION_TYPE), func_decl->name,
           ssprintf("invalid `main` function type, expected `() -> void`"));
  }
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
  bool in_loop = true;
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

  bool in_loop = true;
  ANALYZE_ARRAY(for_loop->stmts);
  OK;
});

ANALYZER(assign, {
  ANALYZE(assign->target, target);
  ANALYZE(assign->expr, expr);

  EXPECT(IS_ONE_OF(assign->target->kind, aNAME, aPTR_DEREF), assign->target,
         strdup("cannot assign to temporary value"));

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

ANALYZER(break_stmt, {
  UNUSED(break_stmt);
  EXPECT(in_loop, node, strdup("cannot break outside of loop"));
  OK;
});

ANALYZER(continue_stmt, {
  UNUSED(continue_stmt);
  EXPECT(in_loop, node, strdup("cannot continue outside of loop"));
  OK;
});

ANALYZER(field, {
  ANALYZE_TYPE(field->type, field);
  node->type = field_type;
  OK;
});

ANALYZER(struct_decl, {
  EXPECT(IS_GLOBAL, node,
         strdup("structs can only be defined in the global scope"));
  EXPECT(is_static, node, strdup("structs can only be defined as constants"));

  DECLARE(struct_decl);
  node->type = PRIM_TYPE(tyTYPE);
  ITER_ARRAY(struct_decl->fields, field_node, ANALYZE(field_node, field));
  OK;
});

ANALYZER(var_decl, {
  bool is_global = IS_GLOBAL;
  bool is_static = is_global || var_decl->is_constant;
  var_decl->is_global = is_global;
  // TODO: structs as global initializers
  EXPECT(!is_global ||
             IS_ONE_OF(var_decl->expr->kind, aINTEGER, aBOOLEAN, aSTRING),
         node, strdup("global initializer must be a literal"));

  DECLARE(var_decl);
  TRY_ANALYZE(var_decl->expr, expr);
  // function type can sometimes be determined even if there are blocking
  // unknowns
  node->type = expr_type;
  if (blocked)
    BLOCK;
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
    ACASE(func_call);
    ACASE(ptr_create);
    ACASE(ptr_deref);
    ACASE(ptr_type);
    ACASE(func_decl);
    ACASE(param);
    ACASE(if_stmt);
    ACASE(while_loop);
    ACASE(for_loop);
    ACASE(assign);
    ACASE(return_stmt);
    ACASE(break_stmt);
    ACASE(continue_stmt);
    ACASE(field);
    ACASE(struct_decl);
    ACASE(field_inst);
    ACASE(struct_inst);
    ACASE(member);
    ACASE(var_decl);
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

void get_analyze_error_info(const char *source, AnalyzeError error,
                            char **out_message, size_t *out_line,
                            size_t *out_column) {
  OffsetInfo oi = get_offset_info(source, error.span.offset);
  *out_message = strdup(error.message);
  *out_line = oi.line_number;
  *out_column = oi.column_number;
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
  State state = {.allocator = &ast->random_allocator, .error_data = error_data};
  Result result = rBLOCKED;
  for (size_t i = 0;; i++) {
    eprintf("INFO: analysis iteration: %zu\n", i);
    // TODO: make sure the symbol tables (also allocated using random_allocator)
    // are freed, but the static values are not
    result = analyze_node(&state, ast->head, &table, (Type){0}, true, false);
    if (result != rBLOCKED)
      break;

    state.error_on_block = state.anything_changed;
    state.anything_changed = false;
  }
  return result;
}
