#pragma once

#include "lexer.h"

typedef struct {
  size_t offset;
  size_t length;
} Span;

typedef enum {
  aINVALID = 0,
  aNAME,
  aINTEGER,
  aBOOLEAN,
  aSTRING,
  aUNARY,
  aBINARY,
  aFUNCTION_CALL,
  aFUNCTION,
  aPARAM,
  aIF_STMT,
  aWHILE_LOOP,
  aFOR_LOOP,
  aASSIGN,
  aRETURN_STMT,
  aFIELD,
  aSTRUCT,
  aFIELD_INST,  // field instantiation
  aSTRUCT_INST, // struct instantiation
  aMEMBER,      // struct member access
  aDECL,        // variable declaration
  aMODULE,
} ASTNodeKind;

typedef struct {
  const char *text;
  size_t length;
} Name;

typedef struct ASTNode ASTNode;

typedef struct {
  ListAllocator list_allocator;
  RandomAllocator random_allocator;
  ASTNode *head;
} AST;

typedef struct {
  ASTNode **data;
  size_t length;
} ASTNodeArray;

typedef enum {
  tyUNKNOWN = 0,
  tyVOID,
  tyINT,
  tyBOOL,
  tySTRING,
  tyFUNCTION,
  tySTRUCT,
  tyUNION,
  tyTYPE,
} TypeKind;

typedef enum {
  NOT_BUILTIN = 0UL,
  PRINT_STRING,
} BuiltinID;

typedef struct Type Type;

typedef struct {
  Type *data;
  size_t length;
} TypeArray;

typedef struct Type {
  TypeKind kind;
  bool is_constant;
  // true if bound to a fixed location in memory
  bool is_bound;
  union {
    Name name;
    struct {
      TypeArray params;
      Type *_return;
    } function;
    // Pointer to the struct definition.
    ASTNode *_struct;
  };
} Type;

typedef struct {
  // Owned string, not zero-delimited.
  char *text;
  size_t length;
} String;

// Currently always pointer-sized so alignment is not an issue.
typedef ssize_t Bool;
typedef ssize_t Int;

typedef union Value Value;

typedef union Value {
  Bool _bool;
  Int _int;
  String string;
  Type type;
  struct {
    // Non-zero if this is a builtin function.
    BuiltinID builtin_id;
    ASTNode *function;
  };
} Value;

typedef struct {
  Name name;
  ASTNode *node;
} Symbol;

typedef struct {
  enum {
    symBUILTIN,
    symSTATIC,
    symDYNAMIC,
  } kind;
  union {
    Value builtin;
    Value *static_ptr;
    size_t local_index;
  };
} SymbolValue;

// Data retrieved from a symbol on query.
typedef struct {
  SymbolValue value;
  Type type;
} SymbolData;

typedef struct SymbolTable SymbolTable;
typedef struct SymbolTable {
  SymbolTable *parent;
  Symbol *data;
  size_t length;
} SymbolTable;

typedef struct ASTNode {
  Span span;
  bool is_analyzed;
  Type type;
  ASTNodeKind kind;
  union {
    struct {
      Name name;
      SymbolValue value;
    } name;
    struct {
      const char *text;
      size_t length;
      Int value;
    } integer;
    struct {
      const char *text;
      size_t length;
      Bool value;
    } boolean;
    struct {
      const char *text;
      size_t length;
      String value;
    } string;
    struct {
      TokenKind op;
      ASTNode *expr;
    } unary;
    struct {
      TokenKind op;
      bool has_parens;
      ASTNode *left;
      ASTNode *right;
    } binary;
    struct {
      ASTNode *function;
      ASTNodeArray args;
    } function_call;
    struct {
      ASTNode *name;
      ASTNode *type;
      // True if the symbol has been added to the declaration table.
      bool symbol_added;
      size_t local_index;
    } param;
    struct {
      ASTNodeArray params;
      ASTNode *return_type; // nullable
      ASTNodeArray stmts;
      SymbolTable table;
      // Number of values in the function's stack frame.
      size_t frame_length;
    } function;
    struct {
      ASTNode *cond;
      ASTNodeArray then_stmts;
      ASTNodeArray else_stmts;
      SymbolTable table;
    } if_stmt;
    struct {
      ASTNode *cond;
      ASTNodeArray stmts;
      SymbolTable table;
    } while_loop;
    struct {
      ASTNode *init;
      ASTNode *cond;
      ASTNode *step;
      ASTNodeArray stmts;
      SymbolTable table;
    } for_loop;
    struct {
      TokenKind op;
      ASTNode *target;
      ASTNode *expr;
    } assign;
    struct {
      ASTNode *expr;
    } return_stmt;
    struct {
      ASTNode *name;
      ASTNode *expr;
    } field_inst;
    struct {
      ASTNode *type;
      ASTNodeArray fields;
    } struct_inst;
    struct {
      ASTNode *expr;
      ASTNode *name;
      size_t field_offset;
      size_t field_length;      
    } member;
    struct {
      ASTNode *name;
      ASTNode *type;
    } field;
    struct {
      ASTNodeArray fields;
      // The total number of values this struct contains,
      // including in nested structs.
      size_t flat_length;
    } _struct;
    struct {
      bool is_constant;
      ASTNode *name;
      ASTNode *expr;
      // True if the symbol has been added to the declaration table.
      bool symbol_added;
      // Value location
      union {
        // Value of the declaration if it is static.
        Value *static_value_ptr;
        // Local index of the declaration if it is dynamic.
        size_t local_index;
      };
    } decl;
    struct {
      ASTNodeArray decls;
      SymbolTable table;
    } module;
  };
} ASTNode;

Span token_span(Token token);
Span join_spans(Span left, Span right);

bool name_eq(Name left, Name right);
bool name_eq_string(Name name, const char *string);
bool type_eq(Type left, Type right);

const char *type_name(TypeKind kind);

void free_ast(AST *ast);

void ast_visit(ASTNode *node, size_t depth, void *callback_data,
               void(callback)(ASTNode *node, size_t depth, void *data));
void ast_print_nodes(ASTNode *node);
const char *ast_node_name(ASTNodeKind kind);

bool add_symbol(RandomAllocator *allocator, SymbolTable *table, Name name,
                ASTNode *node);
bool get_symbol(SymbolTable *table, Name name, SymbolData *out);
SymbolTable get_top_table(SymbolTable table);

// Number of `Value`s large a value of this type is. Flattens structs.
size_t flat_length(Type type);
