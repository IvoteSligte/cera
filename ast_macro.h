#define UPPER_name NAME
#define UPPER_integer INTEGER
#define UPPER_string STRING
#define UPPER_unary UNARY
#define UPPER_binary BINARY
#define UPPER_function_call FUNCTION_CALL
#define UPPER_function FUNCTION
#define UPPER_param PARAM
#define UPPER_for_loop FOR_LOOP
#define UPPER_assign ASSIGN
#define UPPER_return_stmt RETURN_STMT
#define UPPER_declaration DECLARATION
#define UPPER_module MODULE

#define SWITCH(node, cases)                                                    \
  {                                                                            \
    ASTNode *__node = node;                                                    \
    switch (__node->kind) {                                                    \
    case INVALID:                                                              \
      panicf("matched INVALID AST node");                                      \
      cases;                                                                   \
    }                                                                          \
  }

#define CASE(name, ...)                                                        \
  case UPPER_##name: {                                                         \
    __auto_type name = &__node->name;                                           \
    __VA_ARGS__                                                                \
    break;                                                                     \
  }

#define ITER_ARRAY(array, element, ...)                                        \
  {                                                                            \
    size_t element##_index = (array).start_index;                              \
    for (size_t i = 0; i < (array).length; i++) {                              \
      ASTNode *element = &node_array[element##_index];                         \
      __VA_ARGS__;                                                             \
      element##_index += node_array[element##_index].tree_size;                \
    }                                                                          \
  }
