
#include "ast.h"
#include "ast_macro.h"

#define PRIM_DATA($name)                                                       \
  static SymbolData $name##_DATA = {.type = PRIM_TYPE(tyTYPE),                 \
                                    .value = {.type = PRIM_TYPE(ty##$name)},   \
                                    .is_static = true};

PRIM_DATA(VOID);
PRIM_DATA(INT);
PRIM_DATA(BOOL);
PRIM_DATA(STRING);

static SymbolData PRINT_STRING_DATA = (SymbolData){
    .type = {.kind = tyFUNCTION,
             .is_constant = true,
             .function = {.params = {.data = &STRING_DATA.value.type,
                                     .length = 1},
                          ._return = &VOID_DATA.value.type}},
    .value = {.builtin_id = PRINT_STRING},
    .is_static = true};

#define MATCH($name, $NAME)                                                    \
  if (name.length == strlen(#$name) &&                                         \
      strncmp(name.text, #$name, name.length) == 0) {                          \
    *out_data_ptr = &$NAME##_DATA;                                             \
    return true;                                                               \
  }

// NOTE: this currently prevents naming shadowing builtins, even as local
// variables
bool get_builtin(Name name, SymbolData **out_data_ptr) {
  MATCH(void, VOID);
  MATCH(int, INT);
  MATCH(bool, BOOL);
  MATCH(string, STRING);
  MATCH(print_string, PRINT_STRING);
  return false;
}

// Adds a symbol to the table, returning false if NAME was already in the table.
bool add_symbol(RandomAllocator *allocator, SymbolTable *table, Name name,
                SymbolData **out_data_ptr) {
  printf("adding: %.*s\n", (int)name.length, name.text);
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
    return false;
  }
  return get_symbol(table->parent, name, out_data_ptr);
}

SymbolTable get_top_table(SymbolTable table) {
  if (table.parent == NULL)
    return table;
  else
    return get_top_table(*table.parent);
}
