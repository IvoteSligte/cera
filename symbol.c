
#include "ast.h"

#define PRIM(type_kind)                                                        \
  (Type) { .kind = type_kind, .name = {0} }

#define MATCH_PRIMITIVE(primitive, type_kind)                                  \
  if (strncmp(name.text, #primitive, name.length)) {                           \
    const char *text = #primitive;                                             \
    Type type = PRIM(type_kind);                                               \
    *out = (Symbol){.name = {.text = text, .length = strlen(text)},            \
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
bool add_symbol(RandomAllocator *allocator, SymbolTable *table, ASTNode *node, Name name,
                Type type, Value *target) {
  Symbol builtin;
  if (get_builtin(name, &builtin))
    return false;

  for (size_t i = 0; i < table->length; i++) {
    Symbol symbol = table->data[i];
    if (name_eq(symbol.name, name))
      return false;
  }
  table->data =
      ra_realloc(allocator, table->data, sizeof(Symbol) * (table->length + 1));
  table->data[table->length] =
      (Symbol){.name = name, .type = type, .target = target};
  table->length++;
  return true;
}

bool get_symbol(SymbolTable *table, Name name, Symbol *out) {
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

SymbolTable get_top_table(SymbolTable table) {
  if (table.parent == NULL)
    return table;
  else
    return get_top_table(*table.parent);
}
