
#include "ast.h"
#include "ast_macro.h"

#define NEW_NAME($name) ((Name){.text = #$name, .length = sizeof(#$name) - 1})

#define BUILTIN_SYMBOL($NAME, $type, $value)                                   \
  static SymbolData $NAME##_SYMBOL = {                                         \
      .value = {.kind = symBUILTIN, .builtin = $value}, .type = $type}

#define PRIM_SYMBOL($NAME)                                                     \
  BUILTIN_SYMBOL($NAME, PRIM_TYPE(tyTYPE), {.type = PRIM_TYPE(ty##$NAME)})

PRIM_SYMBOL(VOID);
PRIM_SYMBOL(INT);
PRIM_SYMBOL(BOOL);
PRIM_SYMBOL(STRING);

static SymbolData PRINT_STRING_SYMBOL = {
    .value = {.kind = symBUILTIN,
              .builtin = {.function = {.builtin = PRINT_STRING}}},
    .type = {.kind = tyFUNCTION,
             .is_constant = true,
             .function = {.params = {.data = &STRING_SYMBOL.value.builtin.type,
                                     .length = 1},
                          ._return = &VOID_SYMBOL.value.builtin.type}}};

#define MATCH($name, $NAME)                                                    \
  if (name_eq_string(name, #$name)) {                                          \
    *out = $NAME##_SYMBOL;                                                     \
    return true;                                                               \
  }

bool get_builtin(Name name, SymbolData *out) {
  MATCH(void, VOID);
  MATCH(int, INT);
  MATCH(bool, BOOL);
  MATCH(string, STRING);
  MATCH(print_string, PRINT_STRING);
  return false;
}

// Adds a symbol to the table, returning false if NAME was already in the table.
bool add_symbol(RandomAllocator *allocator, SymbolTable *table, Name name,
                ASTNode *node) {
  if (table->parent == NULL) {
    SymbolData builtin;
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
  table->data[table->length] = (Symbol){.name = name, .node = node};
  table->length++;
  return true;
}

static SymbolValue get_node_value(ASTNode *node, bool is_global) {
  SymbolValue out = {0};
  if (node->kind == aDECL) {
    bool is_static = node->decl.is_constant || is_global;
    out.kind = is_static ? symSTATIC : symDYNAMIC;
    if (is_static) {
      out.static_ptr = node->decl.static_value_ptr;
      assert_or(node->type.kind == tyUNKNOWN || out.static_ptr != NULL, {
        eprintf(">> for declaration '%.*s' (%s):\n",
                FMT(node->decl.name->name.name), type_name(node->type.kind));
      });
    } else {
      out.local_index = node->decl.local_index;
    }
  } else {
    assert(node->kind == aPARAM);
    out.kind = symDYNAMIC;
    out.local_index = node->param.local_index;
  }
  return out;
}

bool get_symbol(SymbolTable *table, Name name, SymbolData *out) {
  if (table->parent == NULL && get_builtin(name, out)) {
    return true;
  }
  for (size_t i = 0; i < table->length; i++) {
    if (name_eq(table->data[i].name, name)) {
      Symbol symbol = table->data[i];
      *out = (SymbolData){
          .value = get_node_value(symbol.node, table->parent == NULL),
          .type = symbol.node->type,
      };
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
