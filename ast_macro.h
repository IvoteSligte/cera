#pragma once

#define NODE_name aNAME
#define NODE_int aINT
#define NODE_bool aBOOL
#define NODE_void aVOID
#define NODE_integer aINTEGER
#define NODE_string aSTRING
#define NODE_unary aUNARY
#define NODE_binary aBINARY
#define NODE_function_call aFUNCTION_CALL
#define NODE_function aFUNCTION
#define NODE_param aPARAM
#define NODE_for_loop aFOR_LOOP
#define NODE_assign aASSIGN
#define NODE_return_stmt aRETURN_STMT
#define NODE_declaration aDECLARATION
#define NODE_module aMODULE

#define SWITCH($node, $default, $cases)                                        \
  {                                                                            \
    ASTNode *__node = $node;                                                   \
    switch (__node->kind) {                                                    \
    case aINVALID:                                                             \
      panicf("matched INVALID AST node");                                      \
      $cases;                                                                  \
    default:                                                                   \
      $default;                                                                \
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

#define ITER_ARRAY(first_element, element, ...)                                \
  {                                                                            \
    ASTNode *element = first_element;                                          \
    size_t i = 0;                                                              \
    while (element != NULL) {                                                  \
      {                                                                        \
        __VA_ARGS__;                                                           \
      }                                                                        \
      element = element->next_sibling;                                         \
      i++;                                                                     \
    }                                                                          \
  }

#define USE_AST_MACRO_HEADER
