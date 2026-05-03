
#include "ast.h"
#include "ast_macro.h"

#define NEW_NAME($name) {.text = #$name, .length = sizeof(#$name) - 1}

#define BUILTIN_SYMBOL($name, $NAME, $type, $value)                            \
  static ASTNode $NAME##_NODE = {                                              \
      .type = $type,                                                           \
      .kind = aDECL,                                                           \
      .decl = {.is_constant = true, .static_value = $value}};                  \
  static Symbol $NAME##_SYMBOL = {                                             \
      .name = NEW_NAME($name), .node = &$NAME##_NODE, .is_static = true}

#define PRIM_SYMBOL($name, $NAME)                                              \
  BUILTIN_SYMBOL($name, $NAME, PRIM_TYPE(tyTYPE),                              \
                 {.type = PRIM_TYPE(ty##$NAME)});

PRIM_SYMBOL(void, VOID);
PRIM_SYMBOL(int, INT);
PRIM_SYMBOL(bool, BOOL);
PRIM_SYMBOL(string, STRING);

static ASTNode PRINT_STRING_NODE = {
    .type = {.kind = tyFUNCTION,
             .is_constant = true,
             .function = {.params = {.data =
                                         &STRING_NODE.decl.static_value.type,
                                     .length = 1},
                          ._return = &VOID_NODE.decl.static_value.type}},
    .kind = aDECL,
    .decl = {.is_constant = 1, .static_value = {.builtin_id = PRINT_STRING}}};

static Symbol PRINT_STRING_SYMBOL = {
    .name = NEW_NAME(print_string), .node = &PRINT_STRING_NODE, .is_static = 1};

#define MATCH($NAME)                                                           \
  if (name_eq(name, $NAME##_SYMBOL.name)) {                                    \
    *out = $NAME##_SYMBOL;                                                     \
    return true;                                                               \
  }

bool get_builtin(Name name, Symbol *out) {
  MATCH(VOID);
  MATCH(INT);
  MATCH(BOOL);
  MATCH(STRING);
  MATCH(PRINT_STRING);
  return false;
}

// Adds a symbol to the table, returning false if NAME was already in the table.
bool add_symbol(RandomAllocator *allocator, SymbolTable *table, Name name,
                ASTNode *node, bool is_static, size_t local_index) {
  if (table->parent == NULL) {
    Symbol builtin;
    if (get_builtin(name, &builtin)) {
      return false;
    }
  }
  for (size_t i = 0; i < table->length; i++) {
    Symbol symbol = table->data[i];
    if (name_eq(symbol.name, name)) {
      return false;
    }
  }
  table->data =
      ra_recalloc(allocator, table->data, sizeof(Symbol) * (table->length + 1));
  table->data[table->length] = (Symbol){.name = name,
                                        .is_static = is_static,
                                        .local_index = local_index,
                                        .node = node};
  table->length++;
  return true;
}

bool get_symbol(SymbolTable *table, Name name, Symbol *out) {
  if (table->parent == NULL && get_builtin(name, out)) {
    return true;
  }
  for (size_t i = 0; i < table->length; i++) {
    if (name_eq(table->data[i].name, name)) {
      *out = table->data[i];
      return true;
    }
  }
  if (table->parent == NULL) {
    return false;
  }
  return get_symbol(table->parent, name, out);
}

SymbolTable get_top_table(SymbolTable table) {
  if (table.parent == NULL)
    return table;
  else
    return get_top_table(*table.parent);
}
