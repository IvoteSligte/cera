
#include "analyzer.h"
#include "ast_macro.h"

// TODO: function types

bool type_node_eq(ASTNode *left, ASTNode *right) {
  // only names can be types at the moment
  assert(left->kind == NAME);
  assert(right->kind == NAME);
  return name_eq(left->name, right->name);
}

bool type_eq(Type left, Type right) {
  return (left.kind == right.kind) && type_node_eq(left.node, right.node);
}

#define FAIL(defer...)                                                         \
  {                                                                            \
    defer;                                                                     \
    return false;                                                              \
  }

#define MUST_ANALYZE(index, name, defer...)                                    \
  Type name##_type = {0};                                                      \
  if (!analyze(node_array, index, &name##_type, return_type, table,            \
               error_data))                                                    \
    FAIL(defer);

#define MUST_ANALYZE_ARRAY(start_index, length, defer...)                      \
  ITER_ARRAY(start_index, length, element,                                     \
             { MUST_ANALYZE(element_index, element, defer); });

#define MUST_ANALYZE_BLOCK($table, start_index, length, defer...)              \
  {                                                                            \
    if (!extend_table($table, node_array, start_index, length, error_data))    \
      FAIL(defer);                                                             \
    Table table = *($table);                                                   \
    MUST_ANALYZE_ARRAY(start_index, length, defer);                            \
  }

void add_error(TypeErrorArray *error_data, Span span, char *message) {
  TypeError error = {.span = span, .message = message};
  error_data->data =
      realloc(error_data->data, sizeof(TypeError) * (error_data->length + 1));
  error_data->data[error_data->length] = error;
  error_data->length++;
}

#define EXPECT(condition, $index, $message, defer...)                          \
  if (!(condition)) {                                                          \
    add_error(error_data, node_array[$index].span, $message);                  \
    defer;                                                                     \
    return false;                                                              \
  }

#define OK return true;

bool is_pure_expr(ASTNode *node_array, size_t index) {
  ASTNode *node = &node_array[index];
  ITER_ARRAY(index, node->tree_size, expr, {
    SWITCH(expr, {
      CASE(function_call, { panicf("TODO"); });
    default:;
    });
  });
  return true;
}

bool get_name(ASTNode *node_array, size_t index, Name *out) {
  ASTNode *node = &node_array[index];
  size_t name_index;
  SWITCH(node, {
    CASE(declaration, { name_index = declaration.name; });
    CASE(param, { name_index = param.name; });
  default:
    return false;
  });
  ASTNode *name_node = &node_array[name_index];
  assert(name_node->kind == NAME);
  *out = name_node->name;
  return true;
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

#define MATCH_PRIMITIVE(primitive, type_kind)                                  \
  if (strncmp(name.text, #primitive, name.length)) {                           \
    const char *text = #primitive;                                             \
    Type type = {.kind = type_kind, 0};                                        \
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
bool add_symbol(Table *table, ASTNode *node_array, size_t index,
                TypeErrorArray *error_data) {
  Name name;
  assert(get_name(node_array, index, &name));

  Symbol builtin;
  EXPECT(!get_builtin(name, &builtin), index,
         strdup("declaration has the same name as a builtin"));

  for (size_t i = 0; i < table->length; i++) {
    Symbol symbol = table->data[i];
    EXPECT(!name_eq(symbol.name, name), index, strdup("duplicate declaration"));
  }
  ASTNode *node = &node_array[index];
  Value value = {0};
  if (node->kind == DECLARATION) {
    node->declaration.value;
  }

  table->data = realloc(table->data, sizeof(Symbol) * (table->length + 1));
  table->data[table->length] =
      (Symbol){.name = name, .node = node, .value = value, .type = {...}};
  table->length++;
  return true;
}

static bool get_symbol(Table *table, Name name, Symbol *out) {
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

bool has_symbol(Table *table, Name name) {
  Symbol symbol;
  return get_symbol(table, name, &symbol);
}

bool extend_table(Table *table, ASTNode *node_array, size_t start_index,
                  size_t length, TypeErrorArray *error_data) {
  ITER_ARRAY(start_index, length, node, {
    ASTNode *node = &node_array[node_index];
    if (node->kind == DECLARATION || node->kind == PARAM) {
      if (!add_symbol(table, node_array, node_index, error_data))
        return false;
    }
  });
  return true;
}

Table get_top_table(Table table) {
  if (table.parent == NULL)
    return table;
  else
    return get_top_table(*table.parent);
}

bool analyze_type(ASTNode *node_array, size_t index, Type *out, Table table,
                  TypeErrorArray *error_data) {
  ASTNode *node = &node_array[index];
  SWITCH(node, {
    CASE(name, {
      // NOTE: currently does not have a unique namespace for types and
      // variables
      Symbol symbol = {0};
      EXPECT(get_symbol(&table, name, &symbol), index,
             strdup("undefined type"));
      EXPECT((symbol.value.kind == tyTYPE), index, strdup("not a type"));
      *out = symbol.value.type;
      return true;
    });
  default:
    panicf("Tried to analyze_type a non-type node. Kind: `%s`\n",
           ast_node_name(node->kind));
  });
}

bool analyze(ASTNode *node_array, size_t index, Type *type, Type return_type,
             Table table, TypeErrorArray *error_data) {
  ASTNode *node = &node_array[index];
  SWITCH(node, {
    CASE(name, {
      Symbol *symbol = get_symbol(&table, name);
      EXPECT((symbol != NULL), index, strdup("undefined variable"));
      *type = symbol->type;
      OK;
    });
    CASE(integer, {
      type->kind = tyINT;
      OK;
    });
    CASE(string, {
      type->kind = tySTRING;
      OK;
    });
    CASE(unary, {
      MUST_ANALYZE(unary.expr, expr);
      if (unary.op == tMINUS) {
        EXPECT((expr_type.kind != tyINT), unary.expr,
               strdup("cannot apply unary operator `-` to non-numeric type"));
        OK;
      }
      panicf("Unknown unary operator: `%s`\n", token_display_name(unary.op));
    });
    CASE(binary, {
      MUST_ANALYZE(binary.left, left);
      MUST_ANALYZE(binary.right, right);
      if (IS_ONE_OF(binary.op, tPLUS, tMINUS, tSTAR, tSLASH)) {
        EXPECT(
            (left_type.kind == tyINT), binary.left,
            strdup(
                "cannot apply binary arithmetic operator to non-numeric type"));
        EXPECT(
            (right_type.kind == tyINT), binary.right,
            strdup(
                "cannot apply binary arithmetic operator to non-numeric type"));
        EXPECT((left_type.kind == right_type.kind), index,
               strdup("types on both sides of the binary arithmetic operator "
                      "must be the same"));
        OK;
      }
      panicf("Unknown binary operator: `%s`\n", token_display_name(binary.op));
    });
    CASE(function_call, { OK; });
    CASE(function, {
      Table table = (Table){.parent = &table, 0};

      MUST_ANALYZE_BLOCK(&table, function.params, function.num_params,
                         free(table.data));
      Type return_type = {.kind = tyVOID};
      if (function.has_return_type) {
        MUST_ANALYZE(function.return_type, declared, free(table.data));
        return_type = declared_type;
      }
      MUST_ANALYZE_BLOCK(&table, function.stmts, function.num_stmts,
                         free(table.data));
      free(table.data);
      type->kind = tyFUNCTION;
      type->node = node;
      OK;
    });
    CASE(param, {
      MUST_ANALYZE(param.type, type);
      *type = type_type;
      OK;
    });
    CASE(for_loop, {
      panicf("TODO\n");
      OK;
    });
    CASE(assign, {
      panicf("TODO\n");
      OK;
    });
    CASE(return_stmt, {
      MUST_ANALYZE(return_stmt.expr, expr);
      EXPECT(type_eq(expr_type, return_type), return_stmt.expr,
             strdup("unexpected type"));
      OK;
    });
    CASE(declaration, {
      MUST_ANALYZE(declaration.value, value);
      if ((table.parent == NULL) || declaration.is_constant) {
        // TODO: determine lack of side-effects and compute value
      }
      // TODO: check if this is a constant that refers to a runtime declaration
      // (in a function)
      OK;
    });
    CASE(module, {
      Table table = {0};
      MUST_ANALYZE_BLOCK(&table, module.definitions, module.num_definitions,
                         free(table.data));
      free(table.data);
      OK;
    });
  });
}
