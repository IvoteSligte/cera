
#include "analyzer.h"
#include "ast_macro.h"

typedef struct {
  enum { tyINT, tySTRING, tySTRUCT, tyUNION, tyALIAS } kind;
  ASTNode *node; // NULL for primitive types
} Type;

#define MUST_ANALYZE(index, name)                                              \
  Type name##_type = {0};                                                      \
  if (!analyze(node_array, index, &name##_type, error_data))                   \
    return false;

#define EXPECT(kind, node, $message, ...)                                      \
  if (!IS_ONE_OF(kind, __VA_ARGS__)) {                                         \
    error_data->span = node_array[node].span;                                  \
    error_data->message = $message;                                            \
    return false;                                                              \
  }

typedef struct {
  Name key;
  ASTNode *value;
} Symbol;

typedef struct {
  Symbol *data;
  size_t length;
} Table;

// Adds a symbol to the table, returning true if the key was already in the
// table.
bool add_symbol(Table *table, Name key, ASTNode *value) {
  for (size_t i = 0; i < table->length; i++) {
    Symbol symbol = table->data[i];
    if (name_eq(symbol.key, key)) {
      return true;
    }
  }
  table->data = realloc(table->data, sizeof(Symbol) * (table->length + 1));
  table->data[table->length] = (Symbol){key, value};
  table->length++;
  return false;
}

ASTNode *get_symbol(Table *table, Name key) {
  for (size_t i = 0; i < table->length; i++) {
    Symbol symbol = table->data[i];
    if (name_eq(symbol.key, key)) {
      return symbol.value;
    }
  }
  return NULL;
}

bool analyze(ASTNode *node_array, size_t index, Type *type,
             TypeError *error_data) {
  ASTNode *node = &node_array[index];
  switch (node->kind) {
  case INVALID:
    panicf("Tried to analyze invalid node.\n");
    break;

    CASE(name, {});
    CASE(integer, {});
    CASE(string, {});
    CASE(unary, {
      MUST_ANALYZE(unary.expr, expr);
      if (unary.op == tMINUS) {
        EXPECT(expr_type.kind, unary.expr,
               strdup("cannot apply unary operator `-` to non-numeric type"),
               tyINT);
        return true;
      }
      panicf("Unknown unary operator: `%s`\n", token_display_name(unary.op));
    });
    CASE(binary, {
      MUST_ANALYZE(binary.left, left);
      MUST_ANALYZE(binary.right, right);
      if (IS_ONE_OF(binary.op, tPLUS, tMINUS, tSTAR, tSLASH)) {
        EXPECT(
            left_type.kind, binary.left,
            strdup(
                "cannot apply binary arithmetic operator to non-numeric type"),
            tyINT);
        EXPECT(
            right_type.kind, binary.right,
            strdup(
                "cannot apply binary arithmetic operator to non-numeric type"),
            tyINT);
        EXPECT(left_type.kind, index,
               strdup("types on both sides of the binary arithmetic operator "
                      "must be the same"),
               (right_type.kind));
        return true;
      }
      panicf("Unknown binary operator: `%s`\n", token_display_name(binary.op));
    });
    CASE(function_call, {});
    CASE(function, {});
    CASE(param, {});
    CASE(for_loop, {});
    CASE(assign, {});
    CASE(declaration, {});
    CASE(module, {});
  }
}
