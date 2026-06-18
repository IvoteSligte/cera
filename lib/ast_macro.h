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
#define NODE_func_call aFUNC_CALL
#define NODE_ptr_create aPTR_CREATE
#define NODE_ptr_deref aPTR_DEREF
#define NODE_ptr_type aPTR_TYPE
#define NODE_func_decl aFUNC_DECL
#define NODE_param aPARAM
#define NODE_if_stmt aIF_STMT
#define NODE_while_loop aWHILE_LOOP
#define NODE_for_loop aFOR_LOOP
#define NODE_assign aASSIGN
#define NODE_return_stmt aRETURN_STMT
#define NODE_field aFIELD
#define NODE_struct_decl aSTRUCT_DECL
#define NODE_field_inst aFIELD_INST
#define NODE_struct_inst aSTRUCT_INST
#define NODE_member aMEMBER
#define NODE_var_decl aVAR_DECL
#define NODE_module aMODULE

#define auto __auto_type

#define AS($ptr, $type) (*($type *)($ptr))

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
    __auto_type $element = ($array).data[i];                                   \
    {                                                                          \
      __VA_ARGS__;                                                             \
    }                                                                          \
  }

#define PRIM_TYPE(type_kind)                                                   \
  (Type) { .kind = type_kind, .is_constant = true }

#define FMT_TYPE($type) type_name(($type).kind)

#define USE_AST_MACRO_HEADER
