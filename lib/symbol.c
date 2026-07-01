
#include "ast.h"
#include "ast_macro.h"

#define DEF_PRIM_TYPE($NAME) static Type $NAME##_TYPE = PRIM_TYPE($NAME)

DEF_PRIM_TYPE(PRIM_TYPE);
DEF_PRIM_TYPE(VOID);
DEF_PRIM_TYPE(INT);
DEF_PRIM_TYPE(BOOL);
DEF_PRIM_TYPE(STR);
DEF_PRIM_TYPE(CHAR);
DEF_PRIM_TYPE(U8);

#define DEF_FUNCTION_TYPE($NAME, $type...)                                     \
  Type $NAME = {.kind = tyFUNCTION, .is_constant = true, .function = $type}

DEF_FUNCTION_TYPE(PRINT_BOOL_TYPE, {.params = {.data = &BOOL_TYPE, .length = 1},
                                    ._return = &VOID_TYPE});
DEF_FUNCTION_TYPE(PRINT_INT_TYPE, {.params = {.data = &INT_TYPE, .length = 1},
                                   ._return = &VOID_TYPE});
DEF_FUNCTION_TYPE(PRINT_STR_TYPE, {.params = {.data = &STR_TYPE, .length = 1},
                                   ._return = &VOID_TYPE});
DEF_FUNCTION_TYPE(PRINT_CHAR_TYPE, {.params = {.data = &CHAR_TYPE, .length = 1},
                                    ._return = &VOID_TYPE});
DEF_FUNCTION_TYPE(PRINT_BYTE_TYPE, {.params = {.data = &U8_TYPE, .length = 1},
                                    ._return = &VOID_TYPE});
static Type TWO_STRS[2] = {PRIM_TYPE(STR), PRIM_TYPE(STR)};
DEF_FUNCTION_TYPE(STR_EQ_TYPE, {.params = {.data = TWO_STRS, .length = 2},
                                   ._return = &BOOL_TYPE});

// not using #$name because it maps bool -> "_Bool" instead of "bool" as
// bool is a macro
#define MATCH($name, $NAME, $type)                                             \
  if (name_eq_string(name, $name)) {                                           \
    *out_type = $type;                                                         \
    *out_builtin = b##$NAME;                                                   \
    return true;                                                               \
  }

#define MATCH_TYPE($name, $NAME) MATCH($name, $NAME, PRIM_TYPE_TYPE)

bool get_builtin(Name name, Type *out_type, BuiltinID *out_builtin) {
  MATCH("print_bool", PRINT_BOOL, PRINT_BOOL_TYPE);
  MATCH("print_int", PRINT_INT, PRINT_INT_TYPE);
  MATCH("print_str", PRINT_STR, PRINT_STR_TYPE);
  MATCH("print_char", PRINT_CHAR, PRINT_CHAR_TYPE);
  MATCH("print_byte", PRINT_BYTE, PRINT_BYTE_TYPE);
  MATCH_TYPE("void", VOID);
  // signed integers
  MATCH_TYPE("i8", I8);
  MATCH_TYPE("i16", I16);
  MATCH_TYPE("i32", I32);
  MATCH_TYPE("i64", I64);
  MATCH_TYPE("int", INT);
  // unsigned integers
  MATCH_TYPE("bool", BOOL);
  MATCH_TYPE("u8", U8);
  MATCH_TYPE("u16", U16);
  MATCH_TYPE("u32", U32);
  MATCH_TYPE("u64", U64);
  MATCH_TYPE("uint", UINT);
  // other
  MATCH_TYPE("str", STR);
  MATCH_TYPE("char", CHAR);
  return false;
}

#define TO_TYPE($prim)                                                         \
  case b##$prim:                                                               \
    *out_builtin_type = $prim##_TYPE;                                          \
    return true;

bool builtin_to_type(BuiltinID builtin, Type *out_builtin_type) {
  switch (builtin) {
    TO_TYPE(VOID);
    TO_TYPE(INT);
    TO_TYPE(BOOL);
    TO_TYPE(STR);
  default:
    return false;
  }
}

bool get_builtin_type(Name name, Type *out_builtin_type) {
  Type type_type = {0};
  BuiltinID builtin = 0;
  return get_builtin(name, &type_type, &builtin) &&
         builtin_to_type(builtin, out_builtin_type);
}

// Adds a symbol to the table, returning false if NAME was already in the table.
bool add_symbol(RandomAllocator *allocator, SymbolTable *table, Name name,
                ASTNode *node) {
  if (table->parent == NULL) {
    Type type;
    BuiltinID builtin;
    if (get_builtin(name, &type, &builtin)) {
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

bool get_symbol(ExternMod *extern_mod, SymbolTable *table, Name name,
                Type *out_type, ASTNode **out_node, ExternDecl **out_extern,
                BuiltinID *out_builtin) {
  for (size_t i = 0; i < table->length; i++) {
    if (name_eq(table->data[i].name, name)) {
      *out_type = table->data[i].node->type;
      *out_node = table->data[i].node;
      return true;
    }
  }
  if (table->parent == NULL) {
    for (size_t i = 0; i < extern_mod->decls.length; i++) {
      auto decl = &extern_mod->decls.data[i];
      if (name_eq(decl->name, name)) {
        *out_type = decl->type;
        *out_extern = decl;
        return true;
      }
    }
    if (get_builtin(name, out_type, out_builtin)) {
      return true;
    }
    return false;
  }
  return get_symbol(extern_mod, table->parent, name, out_type, out_node,
                    out_extern, out_builtin);
}

SymbolTable get_top_table(SymbolTable table) {
  if (table.parent == NULL)
    return table;
  else
    return get_top_table(*table.parent);
}
