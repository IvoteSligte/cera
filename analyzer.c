
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

#define DEF_NODE ASTNode *node = &node_array[index]
#define NODE_ARGS ASTNode *node_array, size_t index

#define FAIL(defer...)                                                         \
  {                                                                            \
    defer;                                                                     \
    return false;                                                              \
  }

#define MUST_ANALYZE(index, name, defer...)                                    \
  if (!analyze_node(node_array, index, table, return_type, error_data,         \
                    is_second_pass))                                           \
    FAIL(defer);                                                               \
  Type name##_type = node_array[index].type;

#define MUST_ANALYZE_ARRAY(array, defer...)                                    \
  ITER_ARRAY(array, element, { MUST_ANALYZE(element_index, element, defer); });

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
bool add_symbol(NODE_ARGS, Table *table, Name name, Type type,
                TypeErrorArray *error_data) {
  Symbol builtin;
  EXPECT(!get_builtin(name, &builtin), index,
         strdup("declaration has the same name as a builtin"));

  for (size_t i = 0; i < table->length; i++) {
    Symbol symbol = table->data[i];
    EXPECT(!name_eq(symbol.name, name), index, strdup("duplicate declaration"));
  }
  DEF_NODE;
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

bool analyze_type(NODE_ARGS, Type *out, Table table,
                  TypeErrorArray *error_data) {
  DEF_NODE;
  SWITCH(node, {
    CASE(name, {
      // NOTE: currently does not have a unique namespace for types and
      // variables
      Symbol symbol = {0};
      EXPECT(get_symbol(&table, *name, &symbol), index,
             strdup("undefined type"));
      EXPECT((symbol.value.kind == tyTYPE), index, strdup("not a type"));
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

Result analyze_node(NODE_ARGS, Table *table, Type return_type,
                    TypeErrorArray *error_data, bool is_second_pass) {
  DEF_NODE;
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
    CASE(integer, { OK((Type){.kind = tyINT}); });
    CASE(string, { OK((Type){.kind = tySTRING}); });
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
        EXPECT((left_type.kind == right_type.kind), index,
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
        assert(function_type.node->kind == FUNCTION);
        __auto_type function = &function_type.node->function;

        EXPECT((function_call->args.length == function->params.length), index,
               strdup("argument count mismatch"));
        size_t arg_index = function_call->args.start_index;
        size_t param_index = function->params.start_index;
        for (size_t i = 0; i < function_call->args.length; i++) {
          ASTNode *arg = &node_array[arg_index];
          ASTNode *param = &node_array[param_index];
          EXPECT((type_eq(arg->type, param->type)), arg_index,
                 strdup("argument type mismatch"));
          arg_index += arg->tree_size;
          param_index += param->tree_size;
        }
        if (function->has_return_type) {
          function_return_type = node_array[function->return_type].type;
        }
      }
      OK(function_return_type);
    });
    CASE(function, {
      Table local_table = (Table){.parent = table, 0};
      Table *table = &local_table;

      MUST_ANALYZE_ARRAY(function->params, free(local_table.data));
      Type return_type = {.kind = tyVOID};
      if (function->has_return_type) {
        MUST_ANALYZE(function->return_type, declared, free(local_table.data));
        return_type = declared_type;
      }
      MUST_ANALYZE_ARRAY(function->stmts, free(local_table.data));
      free(local_table.data);
      OK((Type){.kind = tyFUNCTION, .node = node});
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
      OK(return_type);
    });
    CASE(declaration, {
      MUST_ANALYZE(declaration->value, value);

      ASTNode *name_node = &node_array[declaration->name];
      assert(name_node->kind == NAME);
      Name name = name_node->name;
          
      if (!declaration->is_declared) {
        EXPECT(add_symbol(node_array, index, table, name, value_type,
                          error_data),
               index, strdup("duplicate declaration"));
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
      MUST_ANALYZE_ARRAY(module->definitions, free(local_table.data));
      free(local_table.data);
      OK((Type){0});
    });
  });
}

bool analyze(ASTNode *node_array, TypeErrorArray *error_data) {
  Table table = {0};
  Result result = 0;
  // needs two passes so that forward references are also resolved
  for (bool is_second_pass = false; !is_second_pass; is_second_pass = true) {
    Result result = analyze_node(node_array, 0, &table, (Type){0}, error_data,
                                 is_second_pass);
    if (result == ERROR)
      break;
  }
  assert(result != UNFINISHED); // should be finished after 2 passes
  free(table.data);
  return result == OK;
}
