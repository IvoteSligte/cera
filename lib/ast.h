#pragma once

#include "alloc.h"
#include "lexer.h"

// These prevent having this header depend on LLVM.
typedef struct LLVMOpaqueValue *LLVMValueRef;
typedef struct LLVMOpaqueType *LLVMTypeRef;

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
  aCHARACTER,
  aUNARY,
  aBINARY,
  aFUNC_CALL,
  aPTR_CREATE,
  aPTR_DEREF,
  aPTR_TYPE,
  aINDEX_OP,
  aFUNC_DECL,
  aPARAM,
  aIF_STMT,
  aWHILE_LOOP,
  aFOR_LOOP,
  aASSIGN,
  aRETURN_STMT,
  aBREAK_STMT,
  aCONTINUE_STMT,
  aFIELD,
  aSTRUCT_DECL,
  aFIELD_INST,  // field instantiation
  aSTRUCT_INST, // struct instantiation
  aMEMBER,      // struct member access
  aVAR_DECL,    // variable declaration
  aMODULE,
} ASTNodeKind;

typedef struct {
  const char *text;
  size_t length;
} Name;

typedef struct ASTNode ASTNode;

typedef struct {
  ASTNode **data;
  size_t length;
} ASTNodeArray;

typedef enum {
  tyUNKNOWN = 0,
  tyVOID,
  // signed integers
  tyI8,
  tyI16,
  tyI32,
  tyI64,
  tyINT,
  // unsigned integers
  tyBOOL,
  tyU8,
  tyU16,
  tyU32,
  tyU64,
  tyUINT,
  // other
  tySTRING,
  tyCHAR, // 32-bit unicode character
  tyPTR,
  tyFUNCTION,
  tySTRUCT,
  tyUNION,
  tyARRAY,
  tyTYPE,
} TypeKind;

// Builtins that can be referenced by name.
typedef enum {
  NOT_BUILTIN = 0UL,
  bVOID,
  // signed integers
  bI8,
  bI16,
  bI32,
  bI64,
  bINT,
  // unsigned integers
  bBOOL,
  bU8,
  bU16,
  bU32,
  bU64,
  bUINT,
  // other
  bSTRING,
  bCHAR,
  // functions
  bPRINT_BOOL,
  bPRINT_INT,
  bPRINT_STRING,
  bPRINT_CHAR,
  bPRINT_BYTE,  
} BuiltinID;

typedef struct Type Type;

typedef struct {
  Type *data;
  size_t length;
} TypeArray;

typedef struct Type {
  TypeKind kind;
  bool is_constant;
  union {
    // TODO: .int_width?
    Type *pointee_type; // kind == tyPOINTER
    struct {
      TypeArray params;
      Type *_return;
    } function;
    // Pointer to the struct declaration (aDECL).
    ASTNode *_struct;
    Type* element_type; // kind == tyARRAY
  };
} Type;

typedef struct {
  // Owned string, not zero-delimited.
  char *text;
  size_t length;
} String;

typedef union Value Value;

#define SYMBOL_DATA                                                            \
  struct {                                                                     \
    Type type;                                                                 \
    LLVMValueRef llvm_value;                                                   \
  }

typedef SYMBOL_DATA SymbolData;

typedef struct {
  Name name;
  ASTNode *node;
} Symbol;

typedef struct SymbolTable SymbolTable;
typedef struct SymbolTable {
  SymbolTable *parent;
  Symbol *data;
  size_t length;
} SymbolTable;

typedef struct {
  Name name;
  Type type;
  LLVMValueRef llvm_value;
} ExternDecl;

typedef struct {
  struct {
    ExternDecl *data;
    size_t length;
  } decls;
} ExternMod;

typedef struct ASTNode {
  Span span;
  bool is_analyzed;
  Type type;
  // LLVM representation of the value this node represents.
  // For declarations this is a *pointer* to the value.
  LLVMValueRef llvm_value;
  ASTNodeKind kind;
  union {
    struct {
      Name name;
      // Exactly one of these must be non-zero after analysis.
      ASTNode *decl;
      ExternDecl *extern_decl;
      BuiltinID builtin;
    } name;
    struct {
      const char *text;
      size_t length;
      ssize_t value;
    } integer;
    struct {
      const char *text;
      size_t length;
      bool value;
    } boolean;
    struct {
      const char *text;
      size_t length;
      String value;
    } string;
    struct {
      const char *text;
      size_t length;
      uint32_t value;
    } character;
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
    } func_call;
    struct {
      ASTNode *expr;
    } ptr_create;
    struct {
      ASTNode *expr;
    } ptr_deref;
    struct {
      ASTNode *expr;
    } ptr_type;
    struct {
      ASTNode *expr;
      ASTNode *index;
    } index_op;
    struct {
      ASTNode *name;
      ASTNode *type;

      // True if the symbol has been added to the declaration table.
      bool symbol_added;
    } param;
    struct {
      ASTNode *name;
      ASTNodeArray params;
      ASTNode *return_type; // nullable

      // Forward declarations do not have bodies.
      // They are expected to be resolved at link time.
      bool is_forward_decl;

      ASTNodeArray stmts;
      SymbolTable table;

      // True if the symbol has been added to the declaration table.
      bool symbol_added;
    } func_decl;
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
    } break_stmt;
    struct {
    } continue_stmt;
    struct {
      ASTNode *name;
      ASTNode *expr;
      // Index of the field in the struct declaration.
      size_t index;
    } field_inst;
    struct {
      ASTNode *type;
      ASTNodeArray fields;
    } struct_inst;
    struct {
      ASTNode *expr;
      ASTNode *name;
      // Index of the field within the struct.
      size_t field_index;
    } member;
    struct {
      ASTNode *name;
      ASTNode *type;
    } field;
    struct {
      ASTNode *name;
      ASTNodeArray fields;
      LLVMTypeRef llvm_type;

      // True if the symbol has been added to the declaration table.
      bool symbol_added;
    } struct_decl;
    struct {
      ASTNode *name;
      ASTNode *expr;
      bool is_constant;
      bool is_global;
      // True if the symbol has been added to the declaration table.
      bool symbol_added;
    } var_decl;
    struct {
      ASTNodeArray decls;
      SymbolTable table;
    } module;
  };
} ASTNode;

typedef struct {
  // Used to allocate nodes during parsing.
  ListAllocator list_allocator;
  // Used to allocate types and arrays during analysis.
  RandomAllocator random_allocator;
  ExternMod extern_mod;
  ASTNode *head;
} AST;

Span token_span(Token token);
Span join_spans(Span left, Span right);

char *name_dup_to_string(Name name);

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
bool get_builtin(Name name, Type *out_type, BuiltinID *out_builtin);
bool get_builtin_type(Name name, Type *out_builtin_type);

// Returns true if the symbol with the given name exists.
// Fills out_type and either out_node or out_builtin depending on whether the
// symbol is user-defined or builtin.
bool get_symbol(ExternMod *extern_mod, SymbolTable *table, Name name,
                Type *out_type, ASTNode **out_node, ExternDecl **out_extern,
                BuiltinID *out_builtin);

extern Type PRINT_BOOL_TYPE;
extern Type PRINT_INT_TYPE;
extern Type PRINT_STRING_TYPE;
extern Type PRINT_CHAR_TYPE;
extern Type PRINT_BYTE_TYPE;
// intrinsic
extern Type STRING_EQ_TYPE;

bool is_integer(TypeKind type);
bool is_numeric(TypeKind type);
bool is_comparable(TypeKind type);
bool is_signed(TypeKind type);
