#pragma once

#define NODE_name aNAME
#define NODE_int aINT
#define NODE_bool aBOOL
#define NODE_void aVOID
#define NODE_integer aINTEGER
#define NODE_boolean aBOOLEAN
#define NODE_string aSTRING
#define NODE_unary aUNARY
#define NODE_binary aBINARY
#define NODE_function_call aFUNCTION_CALL
#define NODE_function aFUNCTION
#define NODE_param aPARAM
#define NODE_if_stmt aIF_STMT
#define NODE_while_loop aWHILE_LOOP
#define NODE_for_loop aFOR_LOOP
#define NODE_assign aASSIGN
#define NODE_return_stmt aRETURN_STMT
#define NODE_decl aDECL
#define NODE_module aMODULE

#define SWITCH($node, $cases)                                                  \
  {                                                                            \
    ASTNode *__node = $node;                                                   \
    switch (__node->kind) {                                                    \
    case aINVALID: {                                                           \
      panicf("matched INVALID AST node");                                      \
    }                                                                          \
      $cases;                                                                  \
    }                                                                          \
  }

#define CASE($name, ...)                                                       \
  case NODE_##$name: {                                                         \
    __auto_type $name = &__node->$name;                                        \
    UNUSED($name);                                                             \
    {                                                                          \
      __VA_ARGS__;                                                             \
    }                                                                          \
    break;                                                                     \
  }

#define ITER_ARRAY($array, $element, ...)                                      \
  for (size_t i = 0; i < ($array).length; i++) {                               \
    ASTNode *$element = ($array).data[i];                                      \
    {                                                                          \
      __VA_ARGS__;                                                             \
    }                                                                          \
  }

#define PRIM_TYPE(type_kind)                                                   \
  (Type) { .kind = type_kind, .is_constant = true }

#define FMT_TYPE($type) type_name(($type).kind)

#define USE_AST_MACRO_HEADER
