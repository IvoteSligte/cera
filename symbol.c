
#include "ast.h"

#define PRIM_TYPE(type_kind)                                                   \
  (Type) { .kind = type_kind, .name = {0} }

#define PRIM_DATA($name)                                                       \
  static SymbolData $name##_DATA = {.type = {.kind = tyTYPE, .name = {0}},     \
                                    .value = {.type = PRIM_TYPE(ty##$name)},   \
                                    .is_static = true};

PRIM_DATA(VOID);
PRIM_DATA(INT);
PRIM_DATA(BOOL);
PRIM_DATA(STRING);

#define MATCH_PRIMITIVE($name, $NAME)                                          \
  if (name.length == strlen(#$name) &&                                         \
      strncmp(name.text, #$name, name.length) == 0) {                          \
    *out_data_ptr = &$NAME##_DATA;                                             \
    return true;                                                               \
  }

bool get_builtin(Name name, SymbolData **out_data_ptr) {
  MATCH_PRIMITIVE(void, VOID);
  MATCH_PRIMITIVE(int, INT);
  MATCH_PRIMITIVE(bool, BOOL);
  MATCH_PRIMITIVE(string, STRING);
  return false;
}

// Adds a symbol to the table, returning false if the key was already in the
// table.
bool add_symbol(RandomAllocator *allocator, SymbolTable *table, Name name,
                SymbolData **out_data_ptr) {
  if (get_builtin(name, out_data_ptr)) {
    return false;
  }

  for (size_t i = 0; i < table->length; i++) {
    Symbol symbol = table->data[i];
    if (name_eq(symbol.name, name)) {
      return false;
    }
  }
  table->data =
      ra_recalloc(allocator, table->data, sizeof(Symbol) * (table->length + 1));
  *out_data_ptr = ra_calloc(allocator, sizeof(SymbolData));
  table->data[table->length] = (Symbol){.name = name, .data = *out_data_ptr};
  table->length++;
  return true;
}

bool get_symbol(SymbolTable *table, Name name, SymbolData **out_data_ptr) {
  if (get_builtin(name, out_data_ptr)) {
    return true;
  }
  for (size_t i = 0; i < table->length; i++) {
    if (name_eq(table->data[i].name, name)) {
      *out_data_ptr = table->data[i].data;
    }
  }
  if (table->parent == NULL) {
    return NULL;
  }
  return get_symbol(table->parent, name, out_data_ptr);
}

SymbolTable get_top_table(SymbolTable table) {
  if (table.parent == NULL)
    return table;
  else
    return get_top_table(*table.parent);
}
