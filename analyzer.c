
#include "analyzer.h"
#include "ast_macro.h"

typedef RandomAllocator Allocator;

#define FAIL(defer...)                                                         \
  {                                                                            \
    defer;                                                                     \
    return false;                                                              \
  }

#define MUST_ANALYZE(node, name, defer...)                                     \
  if (!analyze_node(allocator, node, table, return_type, error_data,           \
                    is_second_pass))                                           \
    FAIL(defer);                                                               \
  Type name##_type = node->type;                                               \
  UNUSED(name##_type);

#define MUST_ANALYZE_ARRAY(array, defer...)                                    \
  ITER_ARRAY(array, element, { MUST_ANALYZE(element, element, defer); });

void add_error(TypeErrorArray *error_data, Span span, char *message) {
  TypeError error = {.span = span, .message = message};
  error_data->data =
      realloc(error_data->data, sizeof(TypeError) * (error_data->length + 1));
  error_data->data[error_data->length] = error;
  error_data->length++;
}

#define EXPECT(condition, node, $message, defer...)                            \
  if (!(condition)) {                                                          \
    add_error(error_data, node->span, $message);                               \
    defer;                                                                     \
    return false;                                                              \
  }

#define OK($type...)                                                           \
  {                                                                            \
    node->type = $type;                                                        \
    node->is_analyzed = true;                                                  \
    return true;                                                               \
  }

typedef struct {
  // not zero-delimited
  char *text;
  size_t length;
} String;

typedef struct {
  TypeKind kind;
  union {
    ssize_t _int;
    String string;
    Type type;
  };
} Value;

typedef struct {
  Name name;
  ASTNode *node;
  Value value;
  Type type;
} Symbol;

typedef struct Table Table;
typedef struct Table {
  Table *parent;
  Symbol *data;
  size_t length;
} Table;

#define PRIM(type_kind)                                                        \
  (Type) { .kind = type_kind, .name = {0} }

#define MATCH_PRIMITIVE(primitive, type_kind)                                  \
  if (strncmp(name.text, #primitive, name.length)) {                           \
    const char *text = #primitive;                                             \
    Type type = PRIM(type_kind);                                               \
    *out = (Symbol){.name = {.text = text, .length = strlen(text)},            \
                    .node = NULL,                                              \
                    .value = {.kind = tyTYPE, .type = type},                   \
                    .type = type};                                             \
    return true;                                                               \
  }

bool get_builtin(Name name, Symbol *out) {
  MATCH_PRIMITIVE(void, tyTYPE);
  MATCH_PRIMITIVE(int, tyTYPE);
  MATCH_PRIMITIVE(string, tyTYPE);
  return false;
}

// Adds a symbol to the table, returning false if the key was already in the
// table.
bool add_symbol(Table *table, ASTNode *node, Name name, Type type,
                TypeErrorArray *error_data) {
  Symbol builtin;
  EXPECT(!get_builtin(name, &builtin), node,
         strdup("declaration has the same name as a builtin"));

  for (size_t i = 0; i < table->length; i++) {
    Symbol symbol = table->data[i];
    EXPECT(!name_eq(symbol.name, name), node, strdup("duplicate declaration"));
  }
  table->data = realloc(table->data, sizeof(Symbol) * (table->length + 1));
  table->data[table->length] =
      (Symbol){.name = name, .node = node, .value = {0}, .type = type};
  table->length++;
  return true;
}

bool get_symbol(Table *table, Name name, Symbol *out) {
  if (get_builtin(name, out)) {
    return true;
  }
  for (size_t i = 0; i < table->length; i++) {
    Symbol *symbol = &table->data[i];
    if (name_eq(symbol->name, name)) {
      return symbol;
    }
  }
  if (table->parent == NULL) {
    return NULL;
  }
  return get_symbol(table->parent, name, out);
}

bool set_symbol_value(Table *table, Name name, Value value) {
  for (size_t i = 0; i < table->length; i++) {
    Symbol *symbol = &table->data[i];
    if (name_eq(symbol->name, name)) {
      symbol->value = value;
      return true;
    }
  }
  return false;
}

Table get_top_table(Table table) {
  if (table.parent == NULL)
    return table;
  else
    return get_top_table(*table.parent);
}

bool analyze_type(ASTNode *node, Type *out, Table table,
                  TypeErrorArray *error_data) {
  SWITCH(node, {
    CASE(name, {
      // NOTE: currently does not have a unique namespace for types and
      // variables
      Symbol symbol = {0};
      EXPECT(get_symbol(&table, *name, &symbol), node,
             strdup("undefined type"));
      EXPECT((symbol.value.kind == tyTYPE), node, strdup("not a type"));
      *out = symbol.value.type;
      return true;
    });
  default:
    panicf("Tried to analyze a non-type node as type. Kind: `%s`\n",
           ast_node_name(node->kind));
  });
}

typedef enum {
  ERROR,
  UNFINISHED,
  OK,
} Result;

Result analyze_node(Allocator *allocator, ASTNode *node, Table *table,
                    Type return_type, TypeErrorArray *error_data,
                    bool is_second_pass) {
  if (node->is_analyzed)
    return true;
  SWITCH(node, {
    CASE(name, {
      Symbol symbol = {0};
      if (!get_symbol(table, *name, &symbol)) {
        return UNFINISHED;
      }
      OK(symbol.type);
    });
    CASE(integer, { OK(PRIM(tyINT)); });
    CASE(string, { OK(PRIM(tySTRING)); });
    CASE(unary, {
      MUST_ANALYZE(unary->expr, expr);
      if (unary->op == tMINUS) {
        EXPECT((expr_type.kind != tyINT), unary->expr,
               strdup("cannot apply unary operator `-` to non-numeric type"));
        OK(expr_type);
      }
      // TODO: set expr_type
      panicf("Unknown unary operator: `%s`\n", token_display_name(unary->op));
    });
    CASE(binary, {
      MUST_ANALYZE(binary->left, left);
      MUST_ANALYZE(binary->right, right);
      if (IS_ONE_OF(binary->op, tPLUS, tMINUS, tSTAR, tSLASH)) {
        EXPECT(
            (left_type.kind == tyINT), binary->left,
            strdup(
                "cannot apply binary arithmetic operator to non-numeric type"));
        EXPECT(
            (right_type.kind == tyINT), binary->right,
            strdup(
                "cannot apply binary arithmetic operator to non-numeric type"));
        EXPECT((left_type.kind == right_type.kind), node,
               strdup("types on both sides of the binary arithmetic operator "
                      "must be the same"));
        OK(left_type);
      }
      panicf("Unknown binary operator: `%s`\n", token_display_name(binary->op));
    });
    CASE(function_call, {
      MUST_ANALYZE(function_call->function, function);
      MUST_ANALYZE_ARRAY(function_call->args);
      EXPECT((function_type.kind == tyFUNCTION), function_call->function,
             strdup("not a function"));
      Type function_return_type = {0};
      {
        __auto_type function = function_type.function;

        ASTNode *arg = function_call->args;
        Type *param_type = function.params;
        size_t i = 0;
        while (arg != NULL || i < function.num_params) {
          EXPECT((arg != NULL && param_type != NULL), node,
                 strdup("argument count mismatch"));
          EXPECT((type_eq(arg->type, *param_type)), arg,
                 strdup("argument type mismatch"));
          arg = arg->next_sibling;
          i++;
        }
        if (function._return != NULL) {
          function_return_type = *function._return;
        }
      }
      OK(function_return_type);
    });
    CASE(function, {
      Table local_table = (Table){.parent = table, 0};
      Table *table = &local_table;
      Type type = {.kind = tyFUNCTION, .function = {0}};
      type.function.params =
          ra_calloc(allocator, sizeof(Type) * function->num_params);

      ITER_ARRAY(function->params, param, {
        MUST_ANALYZE(param, param, free(local_table.data));
        type.function.params[i] = param_type;
      });
      MUST_ANALYZE_ARRAY(function->params, free(local_table.data));

      Type return_type = PRIM(tyVOID);
      if (function->return_type != NULL) {
        MUST_ANALYZE(function->return_type, declared, free(local_table.data));
        type.function._return = ra_calloc(allocator, sizeof(Type));
        *type.function._return = declared_type;
      }
      MUST_ANALYZE_ARRAY(function->stmts, free(local_table.data));
      free(local_table.data);
      OK(type);
    });
    CASE(param, {
      MUST_ANALYZE(param->type, type);
      OK(type_type);
    });
    CASE(for_loop, {
      panicf("TODO\n");
      OK((Type){0});
    });
    CASE(assign, {
      panicf("TODO\n");
      OK((Type){0});
    });
    CASE(return_stmt, {
      MUST_ANALYZE(return_stmt->expr, expr);
      EXPECT(type_eq(expr_type, return_type), return_stmt->expr,
             strdup("unexpected type"));
      OK(PRIM(tyVOID));
    });
    CASE(declaration, {
      MUST_ANALYZE(declaration->value, value);

      assert(declaration->name->kind == NAME);
      Name name = declaration->name->name;

      if (!declaration->is_declared) {
        EXPECT(add_symbol(table, node, name, value_type, error_data), node,
               strdup("duplicate declaration"));
        declaration->is_declared = true;
      }
      if ((table->parent == NULL) || declaration->is_constant) {
        // TODO: determine lack of side-effects and compute value
        Value value = {0};
        set_symbol_value(table, name, value);
      }
      // TODO: check if this is a constant that refers to a runtime declaration
      // (in a function)
      OK(value_type);
    });
    CASE(module, {
      Table local_table = {0};
      Table *table = &local_table;
      MUST_ANALYZE_ARRAY(module->declarations, free(local_table.data));
      free(local_table.data);
      OK(PRIM(tyVOID));
    });
  });
}

bool analyze(AST *ast, TypeErrorArray *error_data) {
  Allocator allocator = {0};
  Table table = {0};
  Result result = 0;
  // needs two passes so that forward references are also resolved
  for (bool is_second_pass = false; !is_second_pass; is_second_pass = true) {
    Result result = analyze_node(&allocator, ast->head, &table, (Type){0},
                                 error_data, is_second_pass);
    if (result == ERROR)
      break;
  }
  assert(result != UNFINISHED); // should be finished after 2 passes
  free(table.data);
  // NOTE: this leaves the type in an undefined state
  // should the allocator be passed along without freeing
  // or should types be zeroed?
  ra_free_all(&allocator);
  return result == OK;
}
