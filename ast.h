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
  aSTRING,
  aUNARY,
  aBINARY,
  aFUNCTION_CALL,
  aFUNCTION,
  aPARAM,
  aFOR_LOOP,
  aASSIGN,
  aRETURN_STMT,
  aDECLARATION, // variable declaration
  aMODULE,
} ASTNodeKind;

typedef struct {
  const char *text;
  size_t length;
} Name;

typedef struct ASTNode ASTNode;

typedef struct {
  ListAllocator allocator;
  ASTNode *head;
} AST;

typedef struct {
  ASTNode **data;
  size_t length;
} ASTNodeArray;

typedef enum {
  tyVOID = 0,
  tyINT,
  tyBOOL,
  tySTRING,
  tyFUNCTION,
  tySTRUCT,
  tyUNION,
  tyALIAS,
  tyTYPE,
} TypeKind;

typedef enum {
  NOT_BUILTIN = 0,
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
  };
} Type;

typedef struct {
  // owned string, not zero-delimited
  char *text;
  size_t length;
} String;

typedef union {
  bool boolean;
  ssize_t integer;
  String string;
  Type type;
  struct {
    // non-zero if this is a builtin function
    BuiltinID builtin_id;
    ASTNode *function;
  };
} Value;

typedef struct {
  Type type;
  Value value;
  bool is_static;
} SymbolData;

typedef struct {
  Name name;
  // storing data separately to keep pointers to it valid when the symbol table
  // is reallocated
  SymbolData *data;
} Symbol;

typedef struct SymbolTable SymbolTable;
typedef struct SymbolTable {
  SymbolTable *parent;
  Symbol *data;
  size_t length;
} SymbolTable;

typedef struct ASTNode {
  Span span;
  enum { sPARSED = 0, sTYPED, sANALYZED } stage;
  ASTNodeKind kind;
  union {
    struct {
      Name name;
      Value *value_ptr;
    } name;
    struct {
      const char *text;
      size_t length;
      ssize_t value;
    } integer;
    struct {
      const char *text;
      size_t length;
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
      Value *value_ptr;
    } param;
    struct {
      ASTNodeArray params;
      ASTNode *return_type; // nullable
      ASTNodeArray stmts;
      SymbolTable table;
    } function;
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
      bool is_constant;
      ASTNode *name;
      ASTNode *expr;
      Value *value_ptr;
    } declaration;
    struct {
      ASTNodeArray declarations;
      SymbolTable table;
    } module;
  };
} ASTNode;

Span token_span(Token token);
Span join_spans(Span left, Span right);

bool name_eq(Name left, Name right);
bool type_eq(Type left, Type right);

const char *type_name(TypeKind kind);

void free_ast(AST *ast);

void ast_visit(ASTNode *node, size_t depth,
               void(callback)(ASTNode *node, size_t depth));
void ast_print_nodes(ASTNode *node);
const char *ast_node_name(ASTNodeKind kind);

bool add_symbol(RandomAllocator *allocator, SymbolTable *table, Name name,
                SymbolData **out_data_ptr);
bool get_symbol(SymbolTable *table, Name name, SymbolData **out_data_ptr);
SymbolTable get_top_table(SymbolTable table);
